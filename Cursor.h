#ifndef CURSOR
#define CURSOR

class Cursor{
public:
  struct Position{
    unsigned x, y;
  };

  void up(unsigned rows = 1);
  void down(unsigned rows = 1);
  void left(unsigned columns = 1);
  void right(unsigned columns = 1);
  void position(Position position);
  Position position() const;

  void column(unsigned column = 1);
};

#endif
