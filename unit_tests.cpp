#include <assert.h>
#include <unordered_set>

#include "unit_tests.h"
#include "board.h"

void RunUnitTests()
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
  b1.Reset();
  b1.SetRow(2, 1, 2, 3, 4);
  b1.ReflectVert();
  b2.Reset();
  b2.SetRow(1, 1, 2, 3, 4);
  assert(b1 == b2);

  // Test Reflect Horizontal
  b1.Reset();
  b1.SetCol(0, 1, 2, 3, 4);
  b1.ReflectHorz();
  b2.Reset();
  b2.SetCol(3, 1, 2, 3, 4);
  assert(b1 == b2);
  b1.Reset();
  b1.SetCol(2, 1, 2, 3, 4);
  b1.ReflectHorz();
  b2.Reset();
  b2.SetCol(1, 1, 2, 3, 4);
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

  // Test SmoothnessScore
  b1.Reset();
  b1.SetRow(0,1,1,2,2);
  assert(b1.SmoothnessScore() == 1);
  b1.SetRow(1,1,1,2,2);
  assert(b1.SmoothnessScore() == 2);
  b1.SetRow(2,1,2,3,4);
  assert(b1.SmoothnessScore() == 9);
  b1.SetRow(3,7,5,0,3);
  assert(b1.SmoothnessScore() == 21);
  b1.SetRow(2,0,0,2,0);
  assert(b1.SmoothnessScore() == 4);

  // Test Corner Score
  /*b1.Reset();
  b1.SetRow(0,1,0,1,0);
  b1.SetRow(1,0,1,0,1);
  assert(b1.CornerScore() == 200);
  b1.RotateCW();
  assert(b1.CornerScore() == 200);
  b1.RotateCW();
  assert(b1.CornerScore() == 200);
  b1.RotateCW();
  assert(b1.CornerScore() == 200);
  b1.ReflectVert();
  assert(b1.CornerScore() == 200);
  b1.ReflectHorz();
  assert(b1.CornerScore() == 200);
  */

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