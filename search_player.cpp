#include "search_player.h"

TileNodeWrapper::TileNodeWrapper(float p, TileNode& n) : node(n), prob(p) {}

Direction SearchPlayer::FindBestMove(const Board& board)
{
	Direction dirs[4];
	int nDirs = board.GetLegalMoves(dirs);
	if (nDirs == 0) return None;

	TileNode root;
	root.board = board;

	for(int i=0; i<nDirs; ++i){
		Direction dir = dirs[i];
		MoveNode kid;
		kid.board = root.board;
		kid.board.Slide(dir);
		root.kids.insert(std::make_pair<int, MoveNode>(dir, kid));
	}

	// TODO

	AccumInfo(root);

	Direction bestDir = None;
	float bestScore = 0.0f;
	for (std::unordered_map<int,MoveNode>::iterator x = root.kids.begin();
		x != root.kids.end(); ++x){
		if (x->second.score > bestScore){
			bestDir = (Direction)x->first;
			bestScore = x->second.score;
		}
	}

	return bestDir;
}

void SearchPlayer::AccumInfo(MoveNode& node) const
{
	if (node.kids.empty()){
		node.score = Eval(node.board);
		return;
	}

	// TODO
}

void SearchPlayer::AccumInfo(TileNode& node) const
{
	if (node.kids.empty()){
		node.score = Eval(node.board);
		return;
	}

	for(std::unordered_map<int,MoveNode>::iterator x = node.kids.begin();
		x != node.kids.end(); ++x) {
		AccumInfo(x->second);
	}

	node.score = -std::numeric_limits<float>::infinity();
	for(std::unordered_map<int,MoveNode>::iterator x = node.kids.begin();
		x != node.kids.end(); ++x) {
		if (x->second.score > node.score)
			node.score = x->second.score;
	}
}

float SearchPlayer::Eval(const Board& board) const
{
	float a = log(board.score);
	float b = board.MaxTile();
	float c = board.NumAvailableTiles();

	return 0.2f*a + 0.3f*b + 0.5f*c;
}