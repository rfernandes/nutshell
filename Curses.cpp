#include "Curses.h"
#include "Command.h"

#include <ncurses.h>

using namespace std;

namespace curses_manip {

color::color(unsigned int pairId)
:_pairId{pairId}
{
}

unsigned color::pairId() const {
  return _pairId;
}

Curses& reset(Curses& curses) {
  standend();
  return curses;
}

} // namespace curses_manip

Curses::Curses() {
  curs_set(1);
}

unsigned Curses::get() {
  return getch();
}

void Curses::refresh() {
  ::refresh();
}

Curses& operator << (Curses& curses, const char * cstr) {
  addstr(cstr);
  return curses;
}

Curses& operator << (Curses& curses, const string& str) {
  addstr(str.data());
  return curses;
}

Curses& operator << (Curses& curses, char ch) {
  addch(ch);
  return curses;
}

Curses& operator << (Curses& curses, const curses_manip::color& obj) {
  attron(COLOR_PAIR(obj.pairId()));
  return curses;
}

Curses& operator << (Curses& curses, Curses& (*pf)(Curses &)) {
  return pf(curses);
}

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


