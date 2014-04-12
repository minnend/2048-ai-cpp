#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "board.h"
#include "rng.h"

static double CPMS = CLOCKS_PER_SEC / 1000.0;

void Test()
{  
  Board b1, b2;
  byte list[16];

  assert(b1.HasOpenTiles());
  assert(b1.NumAvailableTiles() == 16);
  assert(!b1.CanSlideUp());
  assert(!b1.CanSlideRight());
  assert(!b1.CanSlideDown());
  assert(!b1.CanSlideLeft());

  b1.SetRow(0, 1, 2, 0, 1);  
  assert(b1.HasOpenTiles());
  assert(b1.NumAvailableTiles() == 13);
  assert(b1.GetAvailableTiles(list) == 13);

  // test copy
  b2 = b1;
  assert(b1.SlideLeft());
  assert(!b1.SlideLeft());
  assert(b2.SlideLeft());

  // Test column ops
  for(int i=0; i<4; ++i){
    b1.Reset();
    b1.SetCol(i, 1, 2, 3, 4);
    assert(GetCol(i, b1.board) == 0x1234);
    assert(GetReverseCol(i, b1.board) == 0x4321);
  }

  // Test SlideUp
  b1.Reset();
  b1.SetCol(0, 2, 2, 1, 1);
  assert(b1.CanSlideUp());
  assert(b1.SlideUp());    
  b2.Reset();
  b2.SetCol(0, 3, 2, 0, 0);
  assert(b1 == b2);
  assert(!b1.CanSlideUp());
  assert(!b1.SlideUp());
  b1.SetCol(0, 1, 1, 1, 1);
  assert(b1.SlideUp());
  b2.SetCol(0, 2, 2, 0, 0);
  assert(b1 == b2);
  b1.SetCol(0, 1, 2, 3, 4);
  assert(!b1.CanSlideUp());
  assert(!b1.SlideUp());

  b1.Reset();
  b1.SetCol(1, 1, 0, 1, 0);
  assert(b1.CanSlideUp());
  assert(b1.SlideUp());
  b2.Reset();
  b2.SetCol(1, 2, 0, 0, 0);
  assert(b1 == b2);

  // Test SlideRight
  b1.Reset();
  b1.SetRow(0, 2, 2, 1, 1);
  assert(b1.CanSlideRight());
  assert(b1.SlideRight());    
  b2.Reset();
  b2.SetRow(0, 0, 0, 3, 2);
  assert(b1 == b2);
  assert(!b1.CanSlideRight());
  assert(!b1.SlideRight());
  b1.SetRow(0, 1, 1, 1, 1);
  assert(b1.SlideRight());
  b2.SetRow(0, 0, 0, 2, 2);
  assert(b1 == b2);
  b1.SetRow(0, 1, 2, 3, 4);
  assert(!b1.CanSlideRight());
  assert(!b1.SlideRight());
  
  // Test SlideDown
  b1.Reset();
  b1.SetCol(0, 2, 2, 1, 1);
  assert(b1.CanSlideDown());
  assert(b1.SlideDown());    
  b2.Reset();
  b2.SetCol(0, 0, 0, 3, 2);
  assert(b1 == b2);
  assert(!b1.CanSlideDown());
  assert(!b1.SlideDown());
  b1.SetCol(0, 1, 1, 1, 1);
  assert(b1.SlideDown());
  b2.SetCol(0, 0, 0, 2, 2);
  assert(b1 == b2);
  b1.SetCol(0, 1, 2, 3, 4);
  assert(!b1.CanSlideDown());
  assert(!b1.SlideDown());
  
  // Test SlideLeft
  b1.Reset();
  b1.SetRow(0, 2, 2, 1, 1);
  assert(b1.CanSlideLeft());
  assert(b1.SlideLeft());    
  b2.Reset();
  b2.SetRow(0, 3, 2, 0, 0);
  assert(b1 == b2);
  assert(!b1.CanSlideLeft());
  assert(!b1.SlideLeft());
  b1.SetRow(0, 1, 1, 1, 1);
  assert(b1.SlideLeft());
  b2.SetRow(0, 2, 2, 0, 0);
  assert(b1 == b2);
  b1.SetRow(0, 1, 2, 3, 4);
  assert(!b1.CanSlideLeft());
  assert(!b1.SlideLeft());
}

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

Direction FindBestMove(const Board& board)
{
  return (Direction)(rand_uint() & 3);
}

void PlayGame()
{
  Board board = NewGame();
  clock_t start = clock();
  //while (true) {
  for (int i = 0; i < 10; ++i) {
    printf("---------------------------------------\n");
    printf("Board:\n");
    board.Print();
    Direction move = FindBestMove(board);
    if (move == None) break;
    printf("Move: %s\n", DirName[move]);
    board.Slide(move);
    board.Print();
    board.AddRandomTile();
  }
  clock_t stop = clock();
  printf("Final Board:\n");
  board.Print();
  printf("%d  %d\n", 1 << board.MaxTile(), board.Score());
  printf("Time: %.1fms\n", (stop-start)/CPMS);
}

int main(int argc, char* argv[])
{
  Board::Init();
  srand_sse(1234); // TODO

  Test();
  //TimeMoveSpeed();
  PlayGame();

  //printf("Press any key to continue...");
  //getchar();
  return EXIT_SUCCESS;
}
