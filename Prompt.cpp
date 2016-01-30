#include "Prompt.h"

#include <ncurses.h>

Prompt::Prompt()
: _prompt{"NutShell 0.1 >"}
{
}

void Prompt::operator()() {
  init_pair(1, COLOR_RED, -1);
  attron(COLOR_PAIR(1));
  addstr(_prompt.data());
  attroff(COLOR_PAIR(1));
}