#ifndef SHELL_H
#define SHELL_H

#include <command/Directory.h>
#include <command/Function.h>
#include <io/Cursor.h>
#include <io/Input.h>
#include <io/Output.h>
#include <shell/Path.h>
#include <shell/Line.h>

class CommandStore;

class Shell {
public:
  Shell();

  int run();
  void line();
  void debug(unsigned ch, Cursor::Position pos);

  void line() const;

private:

  void prompt();

  CommandStore& _store;
  Line _line;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  Cd &_cd;
  Function &_function;
  bool _exit;
  unsigned _column;

  Command* _match;
};

#endif
