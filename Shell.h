#ifndef SHELL_H
#define SHELL_H

#include "Command.h"
#include "Cursor.h"
#include "History.h"
#include "Input.h"
#include "Output.h"
#include "Path.h"
#include "Prompt.h"

class Command;

class Shell {
public:
  Shell();

  int run();

private:
  CommandStore& _store;
  Prompt _prompt;
  Line _line;
  History _history;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  bool _exit;
};

#endif
