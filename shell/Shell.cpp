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
: _store{CommandStore::instance()}
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
, _utf8Bytes{0}
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

  ModuleStore::instance().initialize();

  // Source ~/nutshellrc
  ifstream config{_cd.home() / ".nutshellrc"};
  if (config) {
    string command;
    while (getline(config, command)) {
      _store.parse(command, _out, true);
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
    for (auto &module: ModuleStore::modules()){
      module->commandExecute(_buffer, *this);
    }

    try{
      const Description executionResult {
        _store.parse(_buffer, _out, true)};

      for (auto &module: ModuleStore::modules()){
        module->commandExecuted(executionResult, *this);
      }

      switch (executionResult.status) {
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
  auto matched = _store.parse(_line, _out, false);
  for (auto &module: ModuleStore::modules()){
    module->lineUpdated(matched, *this);
  }
}

void Shell::prompt() {
  // call Function "directly", instead of going through store
  _function.parse(":prompt", _out, true);
  _column = _cursor.position().x;

  _idx = 0;
}

int Shell::run() {
  prompt();

  Line buffer;

  unsigned keystroke;
  while (!_exit && (keystroke = _in.get())) {

    bool handledKey{false};
    for (auto &module: ModuleStore::modules()){
      if (module->keyPress(keystroke, *this)) {
        handledKey = true;
        break;
      }
    }

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
        _cursor.column(utf8::idx(_line, _idx) + _column);
        break;
      }
      case Input::Left:
        if (_idx > 0){
          _cursor.left();
          while (!utf8::is_utf8(_line[--_idx])){}
        }
        break;
      case Input::Right:
        if (_line[_idx]){
          _cursor.right();
          _idx += utf8::bytes(_line[_idx]);;
        }
        break;
      case Input::Home:
      case Input::End:
        _idx = _line.size();
        _cursor.column(keystroke == Input::Home ? _column: _column + utf8::size(_line));
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
        _cursor.column(utf8::idx(_line, _idx) + _column);
        break;
      }
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
        if (!_utf8Bytes){
          _utf8Bytes += utf8::bytes(keystroke);
        }
        if (!--_utf8Bytes){
          displayLine();
          _cursor.column(utf8::idx(_line, _idx) + _column);
        }
        break;
      }
    }
  }
  return 0;
}

void Shell::output(istream& in){
  _cursor.save();
  auto column = _cursor.position().x;

  auto lastLine{false};

  string line;
  int lines{0};
  while (getline(in, line)){
    _cursor.forceDown();
    lastLine |= _cursor.max().y == _cursor.position().y;
    _cursor.column(_column);
    ++lines;
    _out << line << Color::Reset << Erase::CursorToEnd;
  }

  if (lastLine){
    _cursor.up(lines);
    _cursor.column(column);
  }else{
    _out << "\n" << Erase::CursorToEnd;
    _cursor.restore();
  }
}

