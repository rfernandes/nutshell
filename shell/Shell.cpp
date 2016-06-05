#include "Shell.h"

#include <command/Command.h>
#include <shell/History.h>

#include <experimental/string_view>
#include <fstream>

using namespace std;
using namespace std::experimental;
using namespace manip;

class BuiltIn: public Command
{
  bool matches(const Line& line) const{
    return line.substr(0, line.find_first_of(' ')) == _command;
  }

public:
  using Function = function<Status(const Line&, Output&)>;

  BuiltIn(string command, Function function)
  : _command{move(command)}
  , _function{move(function)}
  {
  }

  ~BuiltIn() override = default;

  Description parse(const Line& line, Output& output, bool execute) override {
    auto matched = matches(line);
    if (matched && execute){
      _function(line, output);
    }

    Description desc;
    if (matched){
      desc.status = Status::Ok;
      desc.segments.push_back({Segment::Type::Builtin, string_view{line}});
    }

    return desc;
  }

private:
  const string _command;
  const Function _function;
};

Shell::Shell()
: _store(CommandStore::instance())
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
, _match{nullptr}
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

  CommandStore::store<BuiltIn>(":cwd",
                               [&](const Line& /*line*/, Output& output){
                                 output << _cd.cwd().string();
                                 return Status::Ok;
                               });

  // Comments are a simple no-op
  CommandStore::store<BuiltIn>("#",
                               [](const Line& /*line*/, Output& /*output*/){
                                 return Status::Ok;
                               });

  // Source ~/nutshellrc
  ifstream config{_cd.home() / ".nutshellrc"};
  if (config) {
    string command;
    while (getline(config, command)) {
      _store.parse(command, _out, true);
    }
  }
}

void Shell::line() {
  _cursor.column(_column);
  auto matched = _store.parse(_line, _out, false);
  if (matched.status != Status::NoMatch){
    const auto view = string_view(_line);
    auto it = view.begin();
    for (const auto& segment: matched.segments){
      if (it != segment.view.begin()){
        _out << view.substr(it - view.begin(), distance(it, segment.view.begin()));
      }
      it = segment.view.end();

      switch (segment.type){
        case Segment::Type::Builtin:
          _out << Color::Cyan;
          break;
        case Segment::Type::Command:
          _out << Color::Green;
          break;
        case Segment::Type::Parameter:
          _out << Color::Blue;
          break;
        case Segment::Type::Argument:
          _out << Color::Magenta;
          break;
        case Segment::Type::String:
          _out << Color::White;
          break;
        default:
          _out << Color::Yellow;
      }
      _out << segment.view;
    }
    _out << Color::Reset;
    if (it != view.end()){
      _out << view.substr(it - view.begin());
    }
  }else{
    _out << Color::Red << _line << Color::Reset;
  }
}

void Shell::prompt() {
  // call Function "directly", instead of going through store
  _function.parse(":prompt", _out, true);
  _column = _cursor.position().x;
}

void Shell::debug(unsigned ch, Cursor::Position position = Cursor::Position{1,1}) {
  auto startPosition = _cursor.position();
  _cursor.position(position);
  _out << Color::Yellow << ch << Color::Reset << ' ';
  _cursor.position(startPosition);
}

int Shell::run() {
  prompt();

  Line buffer;

  auto& history = CommandStore::store<History>();

  unsigned keystroke;
  while (!_exit && (keystroke = _in.get())) {
    switch (keystroke) {
      case '\n':
        _out << '\n';

        buffer += _line;

        if (!buffer.empty()) {
          try{
            const Description executionResult {
              _match ? _match->parse(buffer, _out, true)
                     : _store.parse(buffer, _out, true)};
            switch (executionResult.status) {
              case Status::Ok:
                break;
              case Status::NoMatch:
                _out << "Command not found [" << _line.substr(0, _line.find_first_of(' ')) << "]\n";
                break;
              case Status::Incomplete:
                _function.parse(":prompt_feed", _out, true);
                _column = _cursor.position().x;
                buffer += "\n";
                _line = Line{};
                continue;
            }
          } catch (exception& ex) {
            _out << "Error " << ex.what() << "\n";
          }
          history.add(buffer);
          buffer.clear();
          _line = Line{};
        }
        prompt();
        break;
//       case '\t': { // Tab
//         const auto& suggestions = _store.suggestions(_line);
//         if (!suggestions.empty()) {
//           if (suggestions.size() == 1) {
//             _line = suggestions.at(0);
//           }
//           for (auto& suggestion: suggestions) {
//             _out << " " << suggestion;
//           }
//           _out << "\n";
//         }
//         break;
//       }
      case Input::Backspace:
      case '\b': // Ctrl-H
        if (_line.empty() || _column == _cursor.position().x){
          break;
        }
        _cursor.left();
      case Input::Delete: {
        _line.erase(_cursor.position().x - _column, 1);
        auto push = _cursor.position().x;
        line();
        _out << Erase::CursorToEnd;
        _cursor.column(push);
        break;
      }
      case Input::Left:
        if (_cursor.position().x > _column)
          _cursor.left();
        break;
      case Input::Right:
        if (_cursor.position().x < _column + _line.size())
          _cursor.right();
        break;
      case Input::Up:
      case Input::Down: {
        _line = keystroke == Input::Down ? history.forward(_line) : history.backward(_line);
        line();
        _out << Erase::CursorToEnd;
        break;
      }
      case Input::Home:
      case Input::End:
        _cursor.column(keystroke == Input::Home ? _column: _column + _line.size());
        break;
      case 18: //Ctrl-R
        _out << "History interactive mode unimplemented\n";
        break;
      case 23:{ //Ctrl-W
        auto pos = _cursor.position().x - _column;
        if (pos > 0){
          --pos;
        } else {
          break;
        }
        auto start = _line.find_last_not_of(" ", pos);
        auto idx = _line.find_last_of(" ", start);
        if (idx == string::npos){
          idx = 0;
        } else {
          ++idx;
        }

        _line.erase(idx, pos + 1 - idx);
        line();
        _out << Erase::CursorToEnd;
        _cursor.column(_column + idx);
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

        _line.insert(_cursor.position().x - _column, 1, keystroke);
        auto push = _cursor.position().x + 1;
        line();
        _cursor.column(push);
        break;
      }
    }
  }
  return 0;
}

