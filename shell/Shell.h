#ifndef SHELL_H
#define SHELL_H

#include <command/Directory.h>
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
  void debug(unsigned ch, Cursor::Position pos);

private:

  void prompt();

  CommandStore& _store;
  Line _line;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  Cd &_cd;
  std::string _prompt;
  bool _exit;
  unsigned _column;
};

#endif
