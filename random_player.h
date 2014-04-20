#ifndef __RANDOM_PLAYER_H__
#define __RANDOM_PLAYER_H__

#include "player.h"
#include "rng.h"

class RandomPlayer : public Player
{
public:
	virtual Direction FindBestMove(const Board& board);

private:
  RNG rng;
};

#endif