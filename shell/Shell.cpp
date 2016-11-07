#include "Shell.h"

#include <command/BuiltIn.h>
#include <command/Command.h>
#include <shell/History.h>
#include <shell/Utils.h>
#include <shell/Module.h>

#include <algorithm>
#include <experimental/string_view>
#include <fstream>

using namespace std;
using namespace std::experimental;
using namespace manip;

Shell::Shell()
: _line{ModuleStore::store<LineBuffer>()}
, _commands{CommandStore::instance()}
, _modules{ModuleStore::instance()}
, _out{cout}
, _cd{CommandStore::store<Cd>()}
, _function{
    CommandStore::store<Function>(std::unordered_map<string, string>{
    { ":prompt",
      "\"\x1b[38;2;230;120;150mNutshell\x1b[0m├─┤\x1b[38;2;120;150;230m\"\n"
      ":cwd\n"
      "\"\x1b[0m│ \""},
    { ":prompt2",
      "date"},
    { ":prompt_feed",
      "\"feed: \""}
    })}
, _exit{false}
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
    string command;
    while (getline(config, command)) {
      _commands.parse(command, _out, true);
    }
  }
}

const LineBuffer& Shell::line() const{
  return _line;
}

LineBuffer& Shell::line(){
  return _line;
}

void Shell::line(const LineBuffer& line){
  _line.line(line.line());
}

void Shell::exit(){
  _exit = true;
}

void Shell::executeCommand(const Line& line){
  _out << '\n';
  _buffer += line;

  if (!_buffer.empty()) {
    _modules.commandExecute(line, *this);

    try{
      const ParseResult executionResult {
        _commands.parse(_buffer, _out, true)};

      _modules.commandExecuted(executionResult, *this);

      switch (executionResult.status()) {
        case Status::Ok:
          break;
        case Status::NoMatch:
          _out << "Command not found [" << _line.firstWord() << "]\n";
          break;
        case Status::Incomplete:
          _function.parse(":prompt_feed", _out, true);
          _column = _cursor.position().x;
          _buffer += '\n';
          _line.clear();
          return;
      }
    } catch (exception& ex) {
      _out << "Error " << ex.what() << '\n';
    }

    _buffer.clear();
    _line.clear();
  }
  prompt();
}

void Shell::displayLine() {
  _cursor.column(_column);
  auto matched = _commands.parse(_line.line(), _out, false);
  _modules.lineUpdated(matched, *this);
  _cursor.column(_line.pos() + _column);
}

void Shell::prompt() {
  // call Function "directly", instead of going through store
  _function.parse(":prompt", _out, true);
  _column = _cursor.position().x;
}

int Shell::run() {
  prompt();

  unsigned keystroke;
  unsigned short utf8Bytes{0};
  while (!_exit && (keystroke = _in.get())) {

    // Parse utf codepoints first, so visitors only get whole chars
    if (!utf8Bytes){
      utf8Bytes += utf8::bytes(keystroke);
    }
    if (--utf8Bytes){
      _line.insert(keystroke);
      continue;
    }

    bool handledKey{_modules.keyPress(keystroke, *this)};
    if (handledKey) {
      displayLine();
    }
  }
  return 0;
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

