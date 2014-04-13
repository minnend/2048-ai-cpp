#ifndef __SEARCH_PLAYER_H__
#define __SEARCH_PLAYER_H__

#include "player.h"

class SearchPlayer : public Player
{
	virtual Direction FindBestMove(const Board& board);
};

#endif