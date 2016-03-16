#ifndef SHELL_H
#define SHELL_H

#include <io/Cursor.h>
#include <io/Input.h>
#include <io/Output.h>
#include <shell/Path.h>
#include <shell/Prompt.h>
#include <shell/Line.h>

class CommandStore;

class Shell {
public:
  Shell();

  int run();

private:
  CommandStore& _store;
  Prompt _prompt;
  Line _line;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  bool _exit;
};

#endif
