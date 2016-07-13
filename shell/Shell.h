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
class History;

class Shell {
public:
  Shell();

  int run();
  void line();

private:

  void prompt();

  Line _line;
  CommandStore& _store;
  History& _historu;
  Suggestion _suggestion;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  Cd &_cd;
  Function &_function;
  bool _exit;
  unsigned _column;
  unsigned _idx;
  short _utf8Bytes;

  Command* _match;
};

#endif
