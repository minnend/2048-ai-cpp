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

ushort GetCol(int iCol, const ushort* board)
{
  const int shift = 12 - iCol*4;
  return ((board[0] >> shift) << 12)
    | (((board[1] >> shift) & 0x000F) << 8)
    | (((board[2] >> shift) & 0x000F) << 4)
    | ((board[3] >> shift)  & 0x000F);
}

ushort GetReverseCol(int iCol, const ushort* board)
{
  const int shift = 12 - iCol*4;
  return ((board[3] >> shift) << 12)
    | (((board[2] >> shift) & 0x000F) << 8)
    | (((board[1] >> shift) & 0x000F) << 4)
    | ((board[0] >> shift)  & 0x000F);
}

void SetCol(ushort col, int iCol, ushort* board)
{
  assert(iCol>=0 && iCol<Width);  
  const ushort ColMask = (0xF000 >> (iCol * 4));
  const ushort BoardMask = ~ColMask;  
  const ushort a = col >> 12;
  const ushort b = (col >> 8) & 0x000F;
  const ushort c = (col >> 4) & 0x000F;
  const ushort d = col & 0x000F;
  const int nshift = 12 - iCol*4;
  board[0] = (board[0] & BoardMask) | (a << nshift);
  board[1] = (board[1] & BoardMask) | (b << nshift);
  board[2] = (board[2] & BoardMask) | (c << nshift);
  board[3] = (board[3] & BoardMask) | (d << nshift);  
}

ushort RowVal(ushort row, int x)
{
  return (row >> (12-x*4)) & 0xF;
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
            const ushort from = (ia << 12) | (ib << 8) | (ic << 4) | id;
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
  *row = (b[0] << 12) | (b[1] << 8) | (b[2] << 4) | b[3];
  return true;
}

bool Board::SlideRightSlow(ushort* row, int* score)
{
  byte b[4];
  for(int i=0; i<4; ++i)
    b[i] = RowVal(*row, 3 - i); // reverse order
  if (!SlideLeftSlow(b, score)) return false;
  *row = (b[3] << 12) | (b[2] << 8) | (b[1] << 4) | b[0];
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
  board[iRow] = (a << 12) | (b << 8) | (c << 4) | d;
}

void Board::SetCol(int iCol, int a, int b, int c, int d)
{
  assert(iCol>=0 && iCol<Width);
  const ushort ColMask = (0xF000 >> (iCol * 4));
  const int mask = ~ColMask;
  const int nshift = 12 - iCol*4;
  board[0] = (board[0] & mask) | (a << nshift);
  board[1] = (board[1] & mask) | (b << nshift);
  board[2] = (board[2] & mask) | (c << nshift);
  board[3] = (board[3] & mask) | (d << nshift);
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
  assert(v << 16);

  const ushort ColMask = (0xF000 >> (x * 4));
  const ushort BoardMask = ~ColMask;
  const int nshift = 12 - x*4;
  board[y] = (board[y] & BoardMask) | (v << nshift);
}

bool Board::HasOpenTiles() const
{  
  for(int y=0; y<Height; ++y){
    int row = board[y];
    for(int x=0; x<Width; ++x){
      if ((row & 0x000F) == 0) return true;
      row >>= 4;
    }
  }
  return false;
}

int Board::NumAvailableTiles() const
{
  int n = 0;
  for(int y=0; y<Height; ++y){
    int row = board[y];
    for(int x=0; x<Width; ++x){
      if ((row & 0x000F) == 0) ++n;
      row >>= 4;
    }
  }
  return n;
}

int Board::GetAvailableTiles(byte* list) const
{
  int n = 0;
  int ix = 0;
  for(int y=0; y<Height; ++y){
    ushort row = board[y];
    if ((row & 0xF000) == 0) list[n++] = ix;
    if ((row & 0x0F00) == 0) list[n++] = ix + 1;
    if ((row & 0x00F0) == 0) list[n++] = ix + 2;
    if ((row & 0x000F) == 0) list[n++] = ix + 3;
    ix += 4;
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
    printf("%04x: ", b);
    byte row[4];
    for(int x=0; x<Width; ++x){
      row[(Width-1)-x] = b & 0xF;
      b >>= 4;
    }
    for(int x=0; x<Width; ++x){
      int val = row[x];
      if (val == 0) printf("    .");
      else printf("% 5d", 1 << val);
    }
    printf("\n");
  }
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
    ushort v = GetCol(x, board);
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
    ushort v = GetReverseCol(x, board);
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
  ushort from = GetCol(iCol, board);
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  ::SetCol(to, iCol, board);
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
  ushort from = GetReverseCol(iCol, board);
  ushort to = Board::moveLeftLUT[from];
  if (from == to) return false;
  ::SetCol(Reverse(to), iCol, board);
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

bool Board::operator==(const Board& b) const
{
  if (this == &b) return true;
  for (int y = 0; y < Height; ++y)
    if (board[y] != b.board[y]) return false;
  return true;
}
