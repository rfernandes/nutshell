#include "Output.h"
#include "Command.h"

#include <ncurses.h>

using namespace std;

namespace manip {

color::color(unsigned int pairId)
:_pairId{pairId}
{
}

unsigned color::pairId() const {
  return _pairId;
}

Output& reset(Output& curses) {
  standend();
  return curses;
}

} // namespace curses_manip

Output::Output() {
  curs_set(1);
}

unsigned Output::get() {
  return getch();
}

void Output::refresh() {
  ::refresh();
}

Output& operator << (Output& curses, const string& str) {
  addstr(str.data());
  return curses;
}

Output& operator << (Output& curses, char ch) {
  addch(ch);
  return curses;
}

Output& operator << (Output& curses, const manip::color& obj) {
  attron(COLOR_PAIR(obj.pairId()));
  return curses;
}

Output& operator << (Output& curses, Output& (*pf)(Output &)) {
  return pf(curses);
}
