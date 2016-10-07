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
: _commands{CommandStore::instance()}
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
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _exit = true;
                                 return Status::Ok;
                               });

  CommandStore::store<BuiltIn>(":help",
                               [](const Line& /*line*/, Output& output){
                                 output << "No one can help you (for now)\n";
                                 return Status::Ok;
                               });

  // Comments are a simple no-op
  CommandStore::store<BuiltIn>("#",
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

const Line& Shell::line() const{
  return _line;
}

void Shell::line(const Line& line){
  _line = line;
  _idx = utf8::size(_line);
}

void Shell::executeCommand(const Line& line){
  _out << '\n';
  _buffer += line;

  if (!_buffer.empty()) {
    _modules.commandExecute(line, *this);

    try{
      const Description executionResult {
        _commands.parse(_buffer, _out, true)};

      _modules.commandExecuted(executionResult, *this);

      switch (executionResult.status()) {
        case Status::Ok:
          break;
        case Status::NoMatch:
          _out << "Command not found [" << _line.substr(0, _line.find_first_of(' ')) << "]\n";
          break;
        case Status::Incomplete:
          _function.parse(":prompt_feed", _out, true);
          _column = _cursor.position().x;
          _idx = 0;
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
  auto matched = _commands.parse(_line, _out, false);
  _modules.lineUpdated(matched, *this);
  _cursor.column(utf8::idx(_line, _idx) + _column);
}

void Shell::prompt() {
  // call Function "directly", instead of going through store
  _function.parse(":prompt", _out, true);
  _column = _cursor.position().x;

  _idx = 0;
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
      _line.insert(_idx++, 1, keystroke);
      continue;
    }

    bool handledKey{_modules.keyPress(keystroke, *this)};

    if (handledKey){
      continue;
    }

    switch (keystroke) {
      case '\n':
        executeCommand(_line);
        break;
      case Input::Backspace:
      case '\b': // Ctrl-H
        if (!_idx){
          break;
        }
        _cursor.left();
        while (!utf8::is_utf8(_line[--_idx])){};
      case Input::Delete: {
        _line.erase(_idx, utf8::bytes(_line[_idx]));
        displayLine();
        break;
      }
      case Input::Left:
        if (_idx > 0){
          _cursor.left();
          while (!utf8::is_utf8(_line[--_idx])){}
        }
        break;
      case Input::Right:
        if (_line.size() > _idx){
          _cursor.right();
          _idx += utf8::bytes(_line[_idx]);;
        }
        break;
      case Input::Home:
        _idx = 0;
        _cursor.column(_column);
        break;
      case Input::End:
        _idx = _line.size();
        _cursor.column(_column + utf8::size(_line));
        break;
      case 23:{ //Ctrl-W
        if (!_idx){
          break;
        }
        auto start = _idx;
        while (--_idx && _line[_idx] == ' '){};
        while (_idx-- && _line[_idx] != ' '){};
        if (_idx != 0) ++_idx;
        _line.erase(_idx, start - _idx);
        displayLine();
        break;
      }
      case '\t': // Complete / suggest

        break;
      case 4: //Ctrl-D
        _exit = true;
        break;
      case Input::Unknown: // Unknown special key received
        break; // ignore list
      default: {
        if (keystroke > 0xff) {
          _out << "■";
          continue;
        }
        _line.insert(_idx++, 1, keystroke);
        displayLine();
        break;
      }
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
    _cursor.up(1);
    _cursor.restore();
  }
}

