#include "random_player.h"
#include "rng.h"

Direction RandomPlayer::FindBestMove(const Board& board)
{
	Direction dirs[4];
	int n = board.GetLegalMoves(dirs);
	if (n == 0) return None;
	return dirs[rand_uint() % n];
}