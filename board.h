#ifndef __BOARD_H__
#define __BOARD_H__

#include <vector>
#include "rng.h"

#ifdef WIN32
typedef unsigned long long uint64_t;
#endif

enum Direction { None=-1, Left=0, Right, Up, Down, NumDirections };

extern std::vector<const char*> DirName;

typedef unsigned char byte;
typedef unsigned short ushort;

ushort Reverse(ushort r);
ushort RowVal(ushort row, int x);

class Board
{
public:
  static void Init();

  Board();  

  void Reset();
  void Print() const;
  void PrintSmall() const;

  void SetRow(int iRow, int a, int b, int c, int d);
  void SetCol(int iCol, int a, int b, int c, int d);
  void SetCol(int iCol, ushort v);
  void SetCell(int ix, ushort v);
  void SetCell(int x, int y, ushort v);

  ushort GetCol(int iCol) const;
  ushort GetReverseCol(int iCol) const;

  bool HasOpenTiles() const;
  int NumAvailableTiles() const;
  int GetAvailableTiles(byte* list) const;
  int GetLegalMoves(Direction* moves) const;
  bool AddRandomTile(RNG& rng);

  bool IsDead() const;
  bool CanSlide(Direction dir) const;
  bool Slide(Direction dir);

  bool CanSlideUp() const;
  bool CanSlideRight() const;
  bool CanSlideDown() const;
  bool CanSlideLeft() const;  

  bool SlideUp();
  bool SlideRight();
  bool SlideDown();
  bool SlideLeft();

  bool SlideUp(int iCol);
  bool SlideRight(int iRow);
  bool SlideDown(int iCol);
  bool SlideLeft(int iRow);

  void RotateCW();
  void ReflectVert();
  void ReflectHorz();

  int SmoothnessScore() const;
  int CornerScore() const;
  int CanonicalScore() const;
  Board GetCanonical() const;

  byte MaxTile() const;
  int Score() const { return score; }

  static bool SlideLeftSlow(ushort* row, int* score);
  static bool SlideLeftSlow(byte* p, int* score);

  bool operator==(const Board& other) const;

  ushort board[4];
  int score;

private:
  int CalcCornerScore() const;

  static ushort moveLeftLUT[];    
  static int scoreLeftLUT[];    
};

namespace std {
  template <>
  struct hash<Board>{
    size_t operator()(const Board &b) const {
      return *(size_t*)b.board;
    }
  };
}

#endif