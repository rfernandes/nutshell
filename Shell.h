#ifndef SHELL_H
#define SHELL_H

#include "Command.h"
#include "Cursor.h"
#include "History.h"
#include "Output.h"
#include "Path.h"
#include "Prompt.h"
#include "Terminal.h"

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
  Output _out;
  Cursor _cursor;
  Path _path;
  CommandStore& _store;
  bool _exit;
};

#endif
