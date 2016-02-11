#ifndef SHELL_H
#define SHELL_H

#include "Curses.h"
#include "Terminal.h"
#include "Path.h"
#include "Prompt.h"
#include "History.h"
#include "Directory.h"

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
  Path _path;
  Directory _directory;
  Command& _command;
  bool _exit;
};

#endif