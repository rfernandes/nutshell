#ifndef CURSOR
#define CURSOR

#include <iostream>

class Cursor{

  std::istream& _in;
  std::ostream& _out;

public:
  Cursor(std::istream& input, std::ostream& output);

  struct Position{
    unsigned x, y;
  };

  void up(unsigned rows = 1);
  void forceDown();
  void down(unsigned rows = 1);
  void left(unsigned columns = 1);
  void right(unsigned columns = 1);
  void position(Position position);

  void save();
  void restore();

  Position max();
  Position position() const;

  void column(unsigned column = 1);
};

#endif
