#ifndef __RANDOM_PLAYER_H__
#define __RANDOM_PLAYER_H__

#include "player.h"

class RandomPlayer : public Player
{
	virtual Direction FindBestMove(const Board& board);
};

#endif