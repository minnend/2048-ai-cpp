#ifndef __SEARCH_PLAYER_H__
#define __SEARCH_PLAYER_H__

#include <unordered_set>
#include <unordered_map>
#include "player.h"

class SearchNode;
class MoveNode;
class TileNode;
class TileNodeWrapper;

class SearchNode
{
public:
	SearchNode() : score(0.0f) {}
	Board board;
	float score;
};

// Board state that results from a move
class MoveNode : public SearchNode
{
public:
	std::unordered_map<Board, TileNodeWrapper> kids;
};

// Board state that results from adding a random tile
class TileNode : public SearchNode
{
public:
	std::unordered_map<int, MoveNode> kids;
};

class TileNodeWrapper
{
public:
	TileNodeWrapper(float prob, TileNode &node);
	
	TileNode node;
	float prob;
};

class SearchPlayer : public Player
{
	virtual Direction FindBestMove(const Board &board);

	void AccumInfo(MoveNode& node) const;
	void AccumInfo(TileNode& node) const;
	float Eval(const Board& board) const;
};

#endif