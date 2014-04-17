#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "rng.h"
#include "random_player.h"
#include "search_player.h"
#include "unit_tests.h"

static const double CPMS = CLOCKS_PER_SEC / 1000.0;

void TimeMoveSpeed()
{
  Board b;
  b.SetRow(0, 0,1,1,0);
  b.SetRow(1, 2,2,0,0);
  b.SetRow(2, 1,0,0,1);
  b.SetRow(3, 0,0,0,3);  

  clock_t start = clock();
  const int NumIters = 1000000;
  int score = 0;
  for (int i = 0; i < NumIters; ++i) {    
    Board c;
    c = b; c.Slide(Up); score += c.score;
    c = b; c.Slide(Right); score += c.score;
    c = b; c.Slide(Down); score += c.score;
    c = b; c.Slide(Left); score += c.score;
  }
  clock_t stop = clock();
  printf("Score: %d\n", score);
  printf("Iterations: %d\n", NumIters);
  printf("Time: %0.1fms\n", (stop-start)/CPMS);
}

Board NewGame()
{
  Board b;
  b.AddRandomTile();
  b.AddRandomTile();
  return b;
}

void PlayGame(Player* player)
{
  Board board = NewGame();
  clock_t start = clock();
  int nMoves = 0;
  while (true) {
  //for (int i = 0; i < 10; ++i) {
    printf("---------------------------------------\n");
    printf("Board:\n");
    board.Print();
    Direction move = player->FindBestMove(board);
    if (move == None) break;
    printf("Move: %s\n", DirName[move]);
    assert(board.CanSlide(move));
    board.Slide(move);
    ++nMoves;
    board.Print();
    printf("Score: %d\n", board.Score());
    board.AddRandomTile();
    if (board.IsDead()) break;
    //getchar();
  }
  clock_t stop = clock();
  printf("Final Board (%d moves):\n", nMoves);
  board.Print();
  printf("%d  %d\n", 1 << board.MaxTile(), board.Score());
  printf("Time: %.1fms\n", (stop-start)/CPMS);
}

int main(int argc, char* argv[])
{
  Board::Init();
  srand_sse(1234); // TODO

  RunUnitTests();
  //TimeMoveSpeed();
  std::unique_ptr<Player> player(new SearchPlayer());
  //PlayGame(player.get());

  //printf("Press any key to continue...");
  //getchar();
  return EXIT_SUCCESS;
}
