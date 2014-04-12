#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "board.h"
#include "rng.h"

#define Width 4
#define Height 4

////////////////////////////////////////////////////////////
// Global Declarations

std::vector<const char*> DirName;

ushort Reverse(ushort r)
{
  return (r >> 12) | ((r >> 4) & 0x00F0) | ((r << 4) & 0x0F00) | (r << 12);
}

ushort RowVal(ushort row, int x)
{
  return (row >> (x*4)) & 0xF;
}

////////////////////////////////////////////////////////////
// Static Declarations

ushort Board::moveLeftLUT[65536];
int Board::scoreLeftLUT[65536];

void Board::Init()
{
  DirName.push_back("Up");
  DirName.push_back("Right");
  DirName.push_back("Down");
  DirName.push_back("Left");

  const int MaxTileVal = 15;
  for(int ia=0; ia<=MaxTileVal; ++ia){
    for(int ib=0; ib<=MaxTileVal; ++ib){
      for(int ic=0; ic<=MaxTileVal; ++ic){
        for(int id=0; id<=MaxTileVal; ++id){
            const ushort from = ia | (ib << 4) | (ic << 8) | (id << 12);
            ushort to = from;
            int score = 0;
            Board::SlideLeftSlow(&to, &score);            
            Board::moveLeftLUT[from] = to;
            Board::scoreLeftLUT[from] = score;
        }
      }
    }
  }
}

bool Board::SlideLeftSlow(ushort* row, int* score)
{
  byte b[4];
  for(int i=0; i<4; ++i)
    b[i] = RowVal(*row, i);
  if (!SlideLeftSlow(b, score)) return false;
  *row = b[0] | (b[1] << 4) | (b[2] << 8) | (b[3] << 12);
  return true;
}

bool Board::SlideLeftSlow(byte* row, int* score)
{
  assert(row != NULL);

  bool bMoved = false;
  int xbase = 0;
  for (int x0 = 1; x0 < Width; ++x0) {
    int val = row[x0];
    if (val == 0) continue;
    for (int x = x0 - 1; x >= xbase; --x) {
      if (row[x] == 0) {
        bMoved = true;
        row[x] = val;
        row[x + 1] = 0;
        continue;
      }
      if (row[x] == val) {
        bMoved = true;
        ++row[x];
        if (score != NULL) *score += (1 << row[x]);
        xbase = x + 1;
        row[x + 1] = 0;
      }
      break;
    }
  }  
  return bMoved;
}

////////////////////////////////////////////////////////////

Board::Board()
{
  Reset();
}

void Board::SetRow(int iRow, int a, int b, int c, int d)
{
  assert(iRow>=0 && iRow<Height);
  board[iRow] = (d << 12) | (c << 8) | (b << 4) | a;
}

void Board::SetCol(int iCol, int a, int b, int c, int d)
{
  assert(iCol>=0 && iCol<Width);
  const ushort ColMask = (0x000F << (iCol * 4));
  const int mask = ~ColMask;
  const int nshift = iCol*4;
  board[0] = (board[0] & mask) | (a << nshift);
  board[1] = (board[1] & mask) | (b << nshift);
  board[2] = (board[2] & mask) | (c << nshift);
  board[3] = (board[3] & mask) | (d << nshift);
}

ushort Board::GetCol(int iCol) const
{
  const int shift = iCol*4;
  return ((board[0] >> shift) & 0x000F)
    | (((board[1] >> shift) & 0x000F) << 4)
    | (((board[2] >> shift) & 0x000F) << 8)
    | (((board[3] >> shift)  & 0x000F) << 12);
}

ushort Board::GetReverseCol(int iCol) const
{
  const int shift = iCol*4;
  return ((board[3] >> shift) & 0x000F)
    | (((board[2] >> shift) & 0x000F) << 4)
    | (((board[1] >> shift) & 0x000F) << 8)
    | (((board[0] >> shift)  & 0x000F) << 12);
}

void Board::SetCol(int iCol, ushort col)
{
  assert(iCol>=0 && iCol<Width);
  const int nshift = iCol*4;
  const ushort ColMask = (0x000F << nshift);
  const ushort BoardMask = ~ColMask;  
  const ushort a = col & 0x000F;
  const ushort b = (col >> 4) & 0x000F;
  const ushort c = (col >> 8) & 0x000F;
  const ushort d = col >> 12;
  board[0] = (board[0] & BoardMask) | (a << nshift);
  board[1] = (board[1] & BoardMask) | (b << nshift);
  board[2] = (board[2] & BoardMask) | (c << nshift);
  board[3] = (board[3] & BoardMask) | (d << nshift);  
}

void Board::SetCell(int ix, ushort v)
{
  assert(ix>=0 && ix<16);
  SetCell(ix & 3, ix >> 2, v);
}

void Board::SetCell(int x, int y, ushort v)
{
  assert(x>=0 && x<4);
  assert(y>=0 && y<4);
  assert(v < 16);

  const int nshift = x*4;
  const ushort ColMask = (0x000F << nshift);
  const ushort BoardMask = ~ColMask;
  board[y] = (board[y] & BoardMask) | (v << nshift);
}

bool Board::HasOpenTiles() const
{  
  uint64_t b = *(uint64_t*)board;
  for(int i=0; i<16; ++i){
    if ((b & 0xF) == 0) return true;
    b >>= 4;
  }
  return false;
}

int Board::NumAvailableTiles() const
{
  uint64_t b = *(uint64_t*)board;
  int n = 0;
  for(int i=0; i<16; ++i){
    if ((b & 0xF) == 0) ++n;
    b >>= 4;
  }
  return n;
}

int Board::GetAvailableTiles(byte* list) const
{
  uint64_t b = *(uint64_t*)board;
  int n = 0;
  for(int i=0; i<16; ++i){
    if ((b & 0xF) == 0) list[n++] = i;
    b >>= 4;
  }

  return n;
}

byte Board::MaxTile() const
{
  uint64_t b = *(uint64_t*)board;
  int vmax = 0;
  for(int i=0; i<16; ++i){
    int v = (b & 0xF);
    if (v > vmax) vmax = v;
    b >>= 4;
  }
  return vmax;
}

bool Board::AddRandomTile()
{
  byte list[16];
  int n = GetAvailableTiles(list);
  if (n < 1) return false;
#ifndef NDEBUG
  for(int i=0; i<n; ++i){
    int ix = list[i];
    assert(RowVal(board[ix/4], ix&3) == 0);
  }
#endif
  int ix = list[rand_uint() % n];
  int v = (rand_float() < 0.9 ? 1 : 2);
  SetCell(ix, v);
  return true;
}

void Board::Reset()
{
  score = 0;
  hashCode = 0;
  ::memset(board, 0, Height * sizeof(ushort));
}

void Board::Print() const
{
  for(int y=0; y<Height; ++y){
    ushort b = board[y];
    printf("%04x: ", Reverse(b));
    for(int x=0; x<Width; ++x){
      int val = b & 0xF;
      if (val == 0) printf("     .");
      else printf("% 6d", 1 << val);
      b >>= 4;
    }
    printf("\n");
  }
}

bool Board::IsDead() const
{
  if (NumAvailableTiles() > 0) return false;
  return !CanSlideLeft() && !CanSlideRight() && !CanSlideUp() && !CanSlideDown();
}

bool Board::CanSlide(Direction dir) const
{
  switch(dir){
  case Up: return CanSlideUp();
  case Right: return CanSlideRight();
  case Down: return CanSlideDown();
  case Left: return CanSlideLeft();
  default: return false;
  }  
}

bool Board::Slide(Direction dir)
{
  switch(dir){
  case Up: return SlideUp();
  case Right: return SlideRight();
  case Down: return SlideDown();
  case Left: return SlideLeft();
  default: return false;
  }  
}

bool Board::CanSlideUp() const
{  
  for (int x = 0; x < Width; ++x) {    
    ushort v = GetCol(x);
    if (moveLeftLUT[v] != v) return true;
  }
  return false;
}

bool Board::CanSlideRight() const
{
  for (int y = 0; y < Height; ++y){
    ushort row = Reverse(board[y]);    
    if (moveLeftLUT[row] != row) return true;
  }
  return false;
}

bool Board::CanSlideDown() const
{  
  for (int x = 0; x < Width; ++x) {    
    ushort v = GetReverseCol(x);
    if (moveLeftLUT[v] != v) return true;
  }
  return false;
}

bool Board::CanSlideLeft() const
{
  for (int y = 0; y < Height; ++y){
    ushort row = board[y];
    if (moveLeftLUT[row] != row) return true;
  }
  return false;
}

bool Board::SlideUp()
{
  return SlideUp(0) | SlideUp(1) | SlideUp(2) | SlideUp(3);
}

bool Board::SlideRight()
{
  return SlideRight(0) | SlideRight(1) | SlideRight(2) | SlideRight(3);
}

bool Board::SlideDown()
{
  return SlideDown(0) | SlideDown(1) | SlideDown(2) | SlideDown(3);
}

bool Board::SlideLeft()
{
  return SlideLeft(0) | SlideLeft(1) | SlideLeft(2) | SlideLeft(3);  
}

bool Board::SlideUp(int iCol)
{  
  ushort from = GetCol(iCol);
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  SetCol(iCol, to);
  score += Board::scoreLeftLUT[from];
  return true;
}

bool Board::SlideRight(int iRow)
{
  ushort from = Reverse(board[iRow]);  
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  score += Board::scoreLeftLUT[from];  
  board[iRow] = Reverse(to);
  return true;
}

bool Board::SlideDown(int iCol)
{  
  ushort from = GetReverseCol(iCol);
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  SetCol(iCol, Reverse(to));
  score += Board::scoreLeftLUT[from];
  return true;
}

bool Board::SlideLeft(int iRow)
{
  ushort from = board[iRow];
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  board[iRow] = to; 
  score += Board::scoreLeftLUT[from];
  return true;
}

void Board::RotateCW()
{
  ushort b[Height];
  ::memcpy(b, board, Height*sizeof(short));
  for(int i=0; i<Height; ++i)
    SetCol(3-i, b[i]);
}

void Board::ReflectVert()
{
  ushort t = board[0];
  board[0] = board[3];
  board[3] = t;

  t = board[1];
  board[1] = board[2];
  board[2] = t;
}

bool Board::operator==(const Board& that) const
{
  if (this == &that) return true;
  uint64_t a = *(uint64_t*)board;
  uint64_t b = *(uint64_t*)that.board;
  return a == b;
}
