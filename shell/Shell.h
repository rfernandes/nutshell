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
class ModuleStore;
class History;

class Shell {
public:
  Shell();

  int run();

  void executeCommand(const Line& line);
  void displayLine();
  const Line& line() const;
  void line(const Line& line);

  void output(std::istream& string);

  std::ostream& out();

private:

  void prompt();

  Line _line;
  Line _buffer;
  CommandStore& _commands;
  ModuleStore& _modules;
  Input _in;
  Output& _out;
  Cursor _cursor;
  Path _path;
  Cd &_cd;
  Function &_function;
  bool _exit;
  unsigned _column;
  unsigned _idx;

  Command* _match;
};

#endif
