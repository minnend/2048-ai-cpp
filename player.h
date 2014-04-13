#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "board.h"

class Player
{
public:
	virtual Direction FindBestMove(const Board& board) = 0;	
};

#endif