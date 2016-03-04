#include "Cursor.h"

#include <ncurses.h>

void Cursor::up(unsigned rows)
{
  unsigned y, x;
  getyx(stdscr, y, x);
  ::move(y + rows, x);
}

void Cursor::down(unsigned rows)
{
  unsigned y, x;
  getyx(stdscr, y, x);
  ::move(y - rows, x);
}

void Cursor::left(unsigned columns)
{
  unsigned y, x;
  getyx(stdscr, y, x);
  ::move(y, x - columns);
}

void Cursor::right(unsigned columns)
{
  unsigned y, x;
  getyx(stdscr, y, x);
  ::move(y, x + columns);
}

void Cursor::position(Cursor::Position position)
{
  ::move(position.y, position.x);
}

Cursor::Position Cursor::position() const
{
  Position ret;
  getyx(stdscr, ret.y, ret.x);
  return ret;
}
