#include "Prompt.h"

#include <ncurses.h>

Prompt::Prompt()
: _prompt{"NutShell 0.1α >"}
{
  init_pair(1, COLOR_BLUE, -1);  // ColorPair 1 is for prompt
  init_pair(2, COLOR_GREEN, -1); // ColorPair 2 is for matching commands
}

void Prompt::operator()() const {
  attron(COLOR_PAIR(1));
  addstr(_prompt.data());
  attroff(COLOR_PAIR(1));
}

unsigned Prompt::width() const{
  return _prompt.size(); // FIXME: Only support one line prompts;
}