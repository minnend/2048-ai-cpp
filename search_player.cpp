#include <assert.h>
#include "search_player.h"

MoveNodeMap MoveNode::boards;
TileNodeWrapperMap TileNode::boards;

TileNodeWrapper::TileNodeWrapper(float p, TileNode *n) : node(n), prob(p) {}

void MoveNode::DeleteBoards()
{
	for(MoveNodeMap::iterator it = MoveNode::boards.begin();
		it != MoveNode::boards.end(); ++it) delete it->second;
	MoveNode::boards.clear();
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
		if (b == kids[i]->board) return true;
	}
	return false;
}

void TileNode::DeleteBoards()
{
	for(TileNodeWrapperMap::iterator it = TileNode::boards.begin();
		it != TileNode::boards.end(); ++it) delete it->second.node;
	TileNode::boards.clear();
}

Direction SearchPlayer::FindBestMove(const Board& board)
{
	MoveNodeMap moveNodes;
	std::vector<TileNode*> tileNodes;

	std::vector< std::pair<int,float> > random_tile_info;
	random_tile_info.push_back(std::make_pair<int,float>(1,0.9f));
	random_tile_info.push_back(std::make_pair<int,float>(2,0.1f));

	TileNode *root = new TileNode;
	root->board = board;
	tileNodes.push_back(root);

	Direction dirs[4];
	byte avail[16];
	const int MaxMoveDepth = 3;
	for(int iMove=0; iMove < MaxMoveDepth; ++iMove) {
		printf("Move: %d  TileNodes: %lu\n", iMove+1, tileNodes.size());
		moveNodes.clear();
		for(int iNode=0; iNode<tileNodes.size(); ++iNode) {
			TileNode* node = tileNodes[iNode];
			int nDirs = node->board.GetLegalMoves(dirs);
			for(int i=0; i<nDirs; ++i){
				Direction dir = dirs[i];
				Board b = node->board;
				b.Slide(dir);
				if (node->IsDupBoard(b)) continue;

				MoveNodeMap::iterator it = moveNodes.find(b);
				if (it == moveNodes.end()) {
					MoveNode *kid = new MoveNode;
					kid->board = b;
					node->kids[dir] = kid;
					moveNodes.insert(std::make_pair<Board,MoveNode*>(b, kid));
				} else {
					node->kids[dir] = it->second;
				}
			}
		}

		printf("Move: %d  MoveNodes: %lu\n", iMove+1, moveNodes.size());
		tileNodes.clear();
		for(MoveNodeMap::iterator it = moveNodes.begin(); it != moveNodes.end(); ++it) {
			MoveNode* node = it->second;
			int nAvail = node->board.GetAvailableTiles(avail);
			random_tile_info[0].second = 0.9f / nAvail;
			random_tile_info[1].second = 0.1f / nAvail;
			for(int i=0; i<nAvail; ++i){
				for(int j=0; j<random_tile_info.size(); ++j){
					Board b = node->board;
					b.SetCell(avail[i], random_tile_info[j].first);
					TileNodeWrapperMap::iterator it = node->kids.find(b);
					if (it == node->kids.end()) {
						TileNode *kid = new TileNode;
						kid->board = b;
						node->kids.insert(std::make_pair<Board,TileNodeWrapper>(
							b, TileNodeWrapper(random_tile_info[j].second, kid)));
						tileNodes.push_back(kid);
					} else {
						it->second.prob += random_tile_info[j].second;
					}
				}
			}
		}
	}

	AccumInfo(root);

	Direction bestDir = None;
	float bestScore = 0.0f;
	for(int i=0; i<NumDirections; ++i){
		MoveNode* kid = root->kids[i];
		if (kid == nullptr) continue;
		printf("%s: %.3f\n", DirName[i], kid->score);
		if (kid->score > bestScore){
			bestDir = (Direction)i;
			bestScore = kid->score;
		}
	}

	MoveNode::DeleteBoards();
	TileNode::DeleteBoards();
	return bestDir;
}

void SearchPlayer::AccumInfo(MoveNode *node) const
{
	if (node->accumed) return;
	if (node->kids.empty()){
		node->score = Eval(node->board);
	} else {
		node->score = 0.0f;
		float wsum = 0.0f;
		for(std::unordered_map<Board,TileNodeWrapper>::iterator x = node->kids.begin();
			x != node->kids.end(); ++x) {
			AccumInfo(x->second.node);
			wsum += x->second.prob;
			node->score += x->second.prob * x->second.node->score;
		}
		node->score /= wsum;
	}
	node->accumed = true;
}

void SearchPlayer::AccumInfo(TileNode *node) const
{
	if (node->accumed) return;
	
	int nKids = 0;
	node->score = -std::numeric_limits<float>::infinity();
	for(int i=0; i<NumDirections; ++i){
		MoveNode* kid = node->kids[i];
		if (kid == nullptr) continue;

		++nKids;
		AccumInfo(kid);
		if (kid->score > node->score)
			node->score = kid->score;
	}
	if (nKids == 0)
		node->score = Eval(node->board);
	node->accumed = true;
}

float SearchPlayer::Eval(const Board& board) const
{
	float a = log(board.score);
	float b = board.MaxTile();
	float c = board.NumAvailableTiles();

	return 0.2f*a + 0.3f*b + 0.5f*c;
}