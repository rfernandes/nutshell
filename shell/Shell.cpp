#include "Shell.h"

#include <command/Directory.h>
#include <core/Utils.h>
#include <core/Module.h>
#include <core/BuiltIn.h>
#include <core/Command.h>
#include <shell/History.h>

#include <algorithm>
#include <experimental/string_view>
#include <fstream>

using namespace std;
using namespace manip;

Shell::Shell()
: _commands{CommandStore::instance()}
, _modules{ModuleStore::instance()}
, _in{cin}
, _out{cout}
, _cursor{cin, cout}
, _cd{CommandStore::store<Cd>()}
, _function{CommandStore::store<Function>()}
, _exit{false}
, _column{0}
{
  setlocale(LC_ALL, "");

  CommandStore::store<BuiltIn>(":exit",
                               "Exit nutshell",
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _exit = true;
                                 return Status::Ok;
                               });

  CommandStore::store<BuiltIn>(":help",
                               "Display help",
                               [](const Line& /*line*/, Output& output){
                                 output << "No one can help you (for now)\n";
                                 return Status::Ok;
                               });

  // Comments are a simple no-op
  CommandStore::store<BuiltIn>("#",
                               "Comment",
                               [](const Line& /*line*/, Output& /*output*/){
                                 return Status::Ok;
                               });

  _modules.initialize();

  // Source ~/nutshellrc
  ifstream config{_cd.home() / ".nutshellrc"};
  if (config) {
    script(config);
  }
}

void Shell::exit(){
  _exit = true;
}

ParseResult Shell::handleExecuteCommand(const LineBuffer& line){
  const auto result = _commands.parse(line.line(), _out);
  if (result.command){
      result.command->execute(result.parseResult, _out);
  }else
  {
    // TODO: Handle error
  }
  return result.parseResult;
}

void Shell::executeCommand(const LineBuffer& line){
  _modules.commandExecute(line.line());
  _out << endl;
  const auto executionResult = handleExecuteCommand(line);
  _modules.commandExecuted(executionResult, line.line());
  switch (executionResult.status()) {
    case Status::NoMatch:
      _out << "Command not found [" << line.firstWord() << "]\n";
      [[fallthrough]];
    case Status::Ok:
      prompt();
      break;
    case Status::Incomplete:
      auto promptParseResult = _function.parse(":prompt_feed", _out);
      _function.execute(promptParseResult, _out);
      _column = _cursor.position().x;
      break;
  }
}

void Shell::displayLine(const LineBuffer& line){
  _cursor.column(_column);
  auto matched = _commands.parse(line.line(), _out);
  _modules.lineUpdated(matched.parseResult, line, *this);
  _cursor.column(line.pos() + _column);
}

void Shell::prompt() {
  //TODO: call Function "directly", instead of going through store
  const auto parseResult = _function.parse(":prompt", _out);
  _function.execute(parseResult, _out);
  _column = _cursor.position().x;
}

void Shell::interactive() {
  prompt();

  unsigned keystroke;
  while (!_exit && (keystroke = _in.get())) {
    _modules.keyPress(keystroke, *this);
  }
}

void Shell::script(istream& in) {
  string line;
  while (!_exit && getline(in, line)) {
    handleExecuteCommand(LineBuffer{line});
  }
}

std::ostream & Shell::out(){
  return _out;
}

void Shell::output(istream& in){
  _cursor.save();
  auto column = _cursor.position().x;

  string line;
  int lines{0};
  while (getline(in, line)){
    _cursor.forceDown();
    _cursor.column(_column);
    ++lines;
    _out << line << Color::Reset << Erase::CursorToEnd;
  }

  const auto lastLine{_cursor.max().y == _cursor.position().y};

  if (lastLine){
    _out << Erase::CursorToEnd;
    if (lines){
      _cursor.up(lines);
      _cursor.column(column);
    }
  }else{
    _out << '\n' << Erase::CursorToEnd;
    _cursor.restore();
  }
}

void Shell::clear(int lines){
  _cursor.save();
  _cursor.column(0);

  for (int l{0}; l < lines; ++l){
    _cursor.forceDown();
    _out << Color::Reset << Erase::CursorToEnd;
  }

  if (lines){
    _cursor.up(lines);
  }
  _cursor.restore();
}

