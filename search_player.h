#ifndef __SEARCH_PLAYER_H__
#define __SEARCH_PLAYER_H__

#include <unordered_set>
#include <unordered_map>
#include "player.h"

class SearchNode;
class MoveNode;
class TileNode;
class TileNodeWrapper;

typedef std::unordered_map<Board, MoveNode*> MoveNodeMap;
typedef std::unordered_map<Board, TileNode*> TileNodeMap;
typedef std::unordered_map<Board, TileNodeWrapper> TileNodeWrapperMap;

class SearchNode
{
protected:
	SearchNode() : score(0.0f), accumed(false) {}

public:
	Board board;
	float score;
	bool accumed;
};

// Board state that results from a move.
// The next step is to add a random tile.
class MoveNode : public SearchNode
{
public:
	std::unordered_map<Board, TileNodeWrapper> kids;

	static void DeleteBoards();
	static MoveNodeMap boards;
};

// Board state that results from adding a random tile.
// The next step is to make a move.
class TileNode : public SearchNode
{
public:
	TileNode();
	bool IsDupBoard(const Board& b) const;

	MoveNode* kids[4];
	
	static void DeleteBoards();
	static TileNodeWrapperMap boards;
};

class TileNodeWrapper
{
public:
	TileNodeWrapper(float prob, TileNode *node);

	TileNode *node;
	float prob;
};

class SearchPlayer : public Player
{
	virtual Direction FindBestMove(const Board &board);

	void AccumInfo(MoveNode *node) const;
	void AccumInfo(TileNode *node) const;
	float Eval(const Board& board) const;
};

#endif