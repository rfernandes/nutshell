#include "Curses.h"
#include "Command.h"

#include <ncurses.h>


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

Curses& operator << (Curses& curses, const std::string& str) {
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
