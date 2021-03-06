#ifndef SHELL_H
#define SHELL_H

#include <command/Function.h>
#include <io/Cursor.h>
#include <io/Input.h>
#include <io/Output.h>
#include <shell/Path.h>
#include <shell/Line.h>
#include <shell/LineBuffer.h>

class CommandStore;
class ModuleStore;
class History;
class Cd;

class Shell {
public:
  Shell();

  void interactive();
  void script(std::istream& in);

  void executeCommand(const LineBuffer& line);
  void displayLine(const LineBuffer& line);

  void output(std::istream& string);
  void clear(int lines);

  void exit();

  std::ostream& out();

private:
  ParseResult handleExecuteCommand(const LineBuffer& line);
  void prompt();

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
};

#endif
