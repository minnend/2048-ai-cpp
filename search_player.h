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
	SearchNode() : score(0.0f), probDeath(0.0f), accumed(false) {}
  virtual ~SearchNode(){}

public:
	static void DeleteAllNodes();

	Board board;
	float score;
	float probDeath;
	bool accumed;

	static std::vector<SearchNode*> all;
};

// Board state that results from a move.
// The next step is to add a random tile.
class MoveNode : public SearchNode
{
public:
	static MoveNode* New();

  std::vector<TileNodeWrapper> kids;
};

// Board state that results from adding a random tile.
// The next step is to make a move.
class TileNode : public SearchNode
{
public:
	static TileNode* New();

	TileNode();
	bool IsDupBoard(const Board& b) const;

	MoveNode* kids[4];
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
	float Eval(const Board& board, bool bPrint = false) const;
};

#endif