#include <assert.h>
#include <time.h>
#include "search_player.h"

static const double CPMS = CLOCKS_PER_SEC / 1000.0;

std::vector<SearchNode*> SearchNode::all;

void SearchNode::DeleteAllNodes()
{
	const int n = all.size();
	for(int i=0; i<n; ++i) delete all[i];
	all.clear();
}

TileNodeWrapper::TileNodeWrapper(float p, TileNode *n) : node(n), prob(p) {}

MoveNode* MoveNode::New()
{
	MoveNode* node = new MoveNode;
	all.push_back(node);
	return node;
}

TileNode* TileNode::New()
{
	TileNode* node = new TileNode;
	all.push_back(node);
	return node;
}

TileNode::TileNode()
{
	for(int i=0; i<NumDirections; ++i)
		kids[i] = nullptr;
}

bool TileNode::IsDupBoard(const Board& b) const
{
	for(int i=0; i<NumDirections; ++i){
		if (kids[i] == nullptr) continue;
		if (b == kids[i]->board.GetCanonical()) return true;
	}
	return false;
}

Direction SearchPlayer::FindBestMove(const Board& board)
{
	clock_t start = clock();  
	MoveNodeMap moveNodes;
	std::vector<TileNode*> tileNodes;

	std::vector< std::pair<int,float> > random_tile_info;
	random_tile_info.push_back(std::make_pair<int,float>(1,0.9f));
	random_tile_info.push_back(std::make_pair<int,float>(2,0.1f));

	TileNode *root = TileNode::New();
	root->board = board;
	tileNodes.push_back(root);

	Direction dirs[4];
	byte avail[16];
	const int MaxMoveDepth = 99;
	const int MaxMS = 30;
	int moveDepth = 0;
	for(int iMove=0; iMove < MaxMoveDepth; ++iMove) {
		moveNodes.clear();
		for(unsigned int iNode=0; iNode<tileNodes.size(); ++iNode) {
			TileNode* node = tileNodes[iNode];
			int nDirs = node->board.GetLegalMoves(dirs);
			for(int i=0; i<nDirs; ++i){
				Direction dir = dirs[i];
				Board b = node->board;
				b.Slide(dir);
				Board canonical = b.GetCanonical();
				if (node->IsDupBoard(canonical)) continue;

				MoveNodeMap::iterator it = moveNodes.find(canonical);
				if (it == moveNodes.end()) {
					MoveNode *kid = MoveNode::New();
					kid->board = b;
					node->kids[dir] = kid;
					moveNodes.insert(std::make_pair<Board,MoveNode*>(canonical, kid));
				} else {
					node->kids[dir] = it->second;
				}
			}
		}
		++moveDepth;
		if ((clock() - start)/CPMS >= MaxMS) break;    

		//printf("Move: %d  MoveNodes: %lu\n", iMove+1, moveNodes.size());
		tileNodes.clear();
		for(MoveNodeMap::iterator it = moveNodes.begin(); it != moveNodes.end(); ++it) {
			MoveNode* node = it->second;
			int nAvail = node->board.GetAvailableTiles(avail);
			random_tile_info[0].second = 0.9f / nAvail;
			random_tile_info[1].second = 0.1f / nAvail;
			for(int i=0; i<nAvail; ++i){
				for(unsigned int j=0; j<random_tile_info.size(); ++j){
					TileNode *kid = TileNode::New();
					kid->board = node->board;
					kid->board.SetCell(avail[i], random_tile_info[j].first);
          node->kids.push_back(TileNodeWrapper(random_tile_info[j].second, kid));
					tileNodes.push_back(kid);
				}
			}
		}
		//printf("Move: %d  TileNodes: %lu\n", iMove+1, tileNodes.size());
		if ((clock() - start)/CPMS >= MaxMS) break;
	}

	AccumInfo(root);

	Direction bestDir = None;
	float bestScore = -std::numeric_limits<float>::infinity();
	float bestDeath = std::numeric_limits<float>::infinity();
	for(int i=0; i<NumDirections; ++i){
		MoveNode* kid = root->kids[i];
		if (kid == nullptr) continue;
		//printf("%s: %.1f  %.1f\n", DirName[i], kid->score, kid->probDeath*100.0f);
	  float deathDiff = kid->probDeath - bestDeath;
		if (deathDiff <= -0.01
			|| (fabs(deathDiff) < 0.01 && kid->score > bestScore)) {
			bestScore = kid->score;
			bestDeath = kid->probDeath;
			bestDir = (Direction)i;
		}
	}
	
	printf("Nodes: %lu    move depth: %d\n", SearchNode::all.size(), moveDepth);
	SearchNode::DeleteAllNodes();

	if (bestDir != None){
		Board b = board;
		b.Slide(bestDir);
		Eval(b, true);
	}

	return bestDir;
}

void SearchPlayer::AccumInfo(MoveNode *node) const
{
	if (node->accumed) return;

	if (node->kids.empty()){
		node->score = Eval(node->board);
		assert(!node->board.IsDead());
		assert(node->probDeath == 0.0f);
	} else {
		assert(node->score == 0.0f);
		float wsum = 0.0f;
    for(unsigned int i=0; i<node->kids.size(); ++i){
			const TileNodeWrapper &wrapper = node->kids[i];
			AccumInfo(wrapper.node);
			wsum += wrapper.prob;
			node->score += wrapper.prob * wrapper.node->score;
			node->probDeath += wrapper.prob * wrapper.node->probDeath;
		}
		node->score /= wsum;
		node->probDeath /= wsum;
	}

	node->accumed = true;
}

void SearchPlayer::AccumInfo(TileNode *node) const
{
	if (node->accumed) return;

	int nKids = 0;
	node->score = -std::numeric_limits<float>::infinity();
	node->probDeath = std::numeric_limits<float>::infinity();
	for(int i=0; i<NumDirections; ++i){
		MoveNode* kid = node->kids[i];
		if (kid == nullptr) continue;

		++nKids;
		AccumInfo(kid);
		float deathDiff = kid->probDeath - node->probDeath;
		if (deathDiff <= -0.01
			|| (fabs(deathDiff) < 0.01 && kid->score > node->score)) {
			node->score = kid->score;
			node->probDeath = kid->probDeath;
		}
	}

	if (nKids == 0) {
		node->score = Eval(node->board);
		node->probDeath = (node->board.IsDead() ? 1.0f : 0.0f);
	}

	node->accumed = true;
}

float SearchPlayer::Eval(const Board& board, bool bPrint) const
{
	float a = log((float)board.score);
	float b = (float)board.MaxTile();
	float c = (float)board.NumAvailableTiles();
	float d = (float)board.SmoothnessScore();
	float e = log(board.CornerScore() / 10.0f + 1.0f);

	if (bPrint)
		printf("Eval: %.3f, %.0f, %.0f, %.0f, %.3f\n", a,b,c,d,e);

	return 0.2f*a + 0.3f*b + 0.3f*c - 0.3f*d + 0.5f*e;
}