#ifndef SHELL_H
#define SHELL_H

#include "Command.h"
#include "Curses.h"
#include "Terminal.h"
#include "Path.h"
#include "Prompt.h"
#include "History.h"

class Command;

class Shell {
public:
  Shell();

  int run();

private:
  Terminal _term;
  Prompt _prompt;
  Line _line;
  History _history;
  Curses _curses;
  Cursor _cursor;
  Path _path;
  CommandStore& _store;
  bool _exit;
};

#endif
