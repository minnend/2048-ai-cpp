#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unordered_set>

#include "board.h"
#include "rng.h"
#include "random_player.h"
#include "search_player.h"

static double CPMS = CLOCKS_PER_SEC / 1000.0;

void Test()
{  
  Board b1, b2;

#ifndef NDEBUG
  byte list[16];
#endif

  assert(b1.HasOpenTiles());
  assert(b1.NumAvailableTiles() == 16);
  assert(b1.GetAvailableTiles(list) == 16);
  assert(!b1.CanSlideUp());
  assert(!b1.CanSlideRight());
  assert(!b1.CanSlideDown());
  assert(!b1.CanSlideLeft());

  b1.SetRow(0, 1, 2, 0, 1);
  assert(b1.HasOpenTiles());
  assert(b1.NumAvailableTiles() == 13);
  assert(b1.GetAvailableTiles(list) == 13);

  // test copy
  b1.Reset();
  b1.SetRow(0, 1, 2, 0, 1);
  b2 = b1;
  assert(b1.SlideLeft());
  assert(!b1.SlideLeft());
  assert(b2.SlideLeft());

  // test memory order
  uint64_t v = 0xfedcba9876543210;
  assert((v&0xF)==0);
  assert((v>>60)==15);
  ::memcpy(&b1.board, &v, 4*sizeof(short));
  //b1.Print();
  assert(b1.board[0] == 0x3210);  
  assert(b1.board[1] == 0x7654);  
  assert(b1.board[2] == 0xba98);  
  assert(b1.board[3] == 0xfedc);
  //for(int i=0; i<4; ++i)
  //  printf("Row %d: %04x\n", i, b1.board[i]);
  //for(int i=0; i<4; ++i)
  //  printf("Column %d: %04x\n", i, b1.GetCol(i));
  assert(b1.GetCol(0) == 0xc840);
  assert(b1.GetCol(1) == 0xd951);
  assert(b1.GetCol(2) == 0xea62);
  assert(b1.GetCol(3) == 0xfb73);
  assert(b1.GetReverseCol(0) == 0x048c);
  assert(b1.GetReverseCol(1) == 0x159d);
  assert(b1.GetReverseCol(2) == 0x26ae);
  assert(b1.GetReverseCol(3) == 0x37bf);
  assert(b1.NumAvailableTiles() == 1);
  assert(b1.CanSlideUp());
  assert(!b1.CanSlideRight());
  assert(!b1.CanSlideDown());
  assert(b1.CanSlideLeft());

  // Test column ops
  for(int i=0; i<4; ++i){
    b1.Reset();
    b1.SetCol(i, 1, 2, 3, 4);
    assert(b1.GetCol(i) == 0x4321);
    assert(b1.GetReverseCol(i) == 0x1234);
  }

  // Test Rotate
  b1.Reset();
  b1.SetRow(0, 1, 2, 3, 4);
  b1.RotateCW();
  b2.Reset();
  b2.SetCol(3, 1, 2, 3, 4);
  assert(b1 == b2);

  // Test Reflect Vertical
  b1.Reset();
  b1.SetRow(0, 1, 2, 3, 4);
  b1.ReflectVert();
  b2.Reset();
  b2.SetRow(3, 1, 2, 3, 4);
  assert(b1 == b2);

  // Test hashing
  std::unordered_set<Board> set;
  b1.Reset();
  set.insert(b1);
  assert(set.size() == 1);
  assert(set.find(b1) != set.end());
  b1.Reset();
  b1.SetRow(0,1,2,3,4);
  assert(set.find(b1) == set.end());
  set.insert(b1);
  assert(set.find(b1) != set.end());
  assert(set.size() == 2);

  // Test canonicalization
  b1.Reset();
  b1.SetRow(0,1,2,3,4);
  b2.Reset();
  b2.SetRow(3,1,2,3,4);
  assert(b1.GetCanonical() == b2.GetCanonical());
  b2.Reset();
  b2.SetCol(0,4,3,2,1);
  assert(b1.GetCanonical() == b2.GetCanonical());

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

  std::unique_ptr<Player> player(new SearchPlayer());
  Test();
  //TimeMoveSpeed();
  PlayGame(player.get());

  //printf("Press any key to continue...");
  //getchar();
  return EXIT_SUCCESS;
}
