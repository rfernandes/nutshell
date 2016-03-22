#include "Shell.h"

#include <command/Command.h>
#include <shell/History.h>

using namespace std;
using namespace manip;

class BuiltIn: public Command
{
public:
  using Function = function<Command::Status(const Line&, Output&)>;

  BuiltIn(string command, Function function)
  : _command{move(command)}
  , _function{move(function)}
  {
  }

  ~BuiltIn() override = default;

  bool matches(const Line& line) const override {
    return line.substr(0, line.find_first_of(' ')) == _command;
  }

  Suggestions suggestions(const Line& line) const override {
    return _command.compare(0, line.find_first_of(' '), line) == 0 ? Suggestions{_command} : Suggestions{};
  }

  Command::Status execute(const Line& line, Output& out) override {
    return matches(line) ? _function(line, out) : Command::Status::NoMatch;
  }

private:
  const string _command;
  const Function _function;
};

Shell::Shell()
: _store(CommandStore::instance())
, _out{cout}
, _exit{false}
{
  setlocale(LC_ALL, "");

  CommandStore::store<BuiltIn>("exit",
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _exit = true;
                                 return Command::Status::Ok;
                               });

  CommandStore::store<BuiltIn>("help",
                               [](const Line& /*line*/, Output& output){
                                 output << "No one can help you (for now)\n";
                                 return Command::Status::Ok;
                               });
  _prompt();
}

int Shell::run() {

  auto& history = CommandStore::store<History>();

  unsigned keystroke;
  while (!_exit && (keystroke = _in.get())) {
    switch (keystroke) {
      case '\n':
        _out << '\n';
        try{
          switch (_store.execute(_line, _out)) {
            case Command::Status::NoMatch:
              _out << "Command not found [" << _line.substr(0, _line.find_first_of(' ')) << "]\n";
              break;
            case Command::Status::Ok:
              break;
          }
        } catch (exception& ex) {
          _out << "Error " << ex.what() << "\n";
        }
        if (!_line.empty()) {
          history.add(_line);
          _line = Line{};
        }
        _prompt();
        break;
      case '\t': { // Tab
        const auto& suggestions = _store.suggestions(_line);
        if (!suggestions.empty()) {
          if (suggestions.size() == 1) {
            _line = {suggestions.at(0)};
          }
          for (auto& suggestion: suggestions) {
            _out << " " << suggestion;
          }
          _out << "\n";
        }
        break;
      }
      case Input::Backspace:
      case '\b': // Ctrl-H
        if (_line.empty()) break;
        _cursor.left();
      case Input::Delete:
        _line.pop_back();
        _out << erase(CursorToEnd);
        break;
      case Input::Left:
        if (_cursor.position().x > _prompt.width())
          _cursor.left();
        break;
      case Input::Right:
        if (_cursor.position().x < _prompt.width() + _line.size())
          _cursor.right();
        break;
      case Input::Up:
      case Input::Down: {
        _line = keystroke == Input::Down ? history.forward(_line) : history.backward(_line);
        _cursor.column();
        _prompt();
        auto matched = _store.matches(_line);
        if (matched){
          _out << color(Green);
        }
        _out << _line;
        if (matched){
          _out << color(Reset);
        }
        _out << erase(CursorToEnd);
        break;
      }
      case Input::Home:
      case Input::End: {
        auto column = keystroke == Input::Home ? _prompt.width()
                                               : _prompt.width() + _line.size();
        _cursor.column(column);
        break;
      }
      case 18: //Ctrl-R
        _out << "History interactive mode unimplemented\n";
        break;
      case 4: //Ctrl-D
        _exit = true;
        break;
      case Input::Unknown: // Unknown special key received
        break; // ignore list
      default: {
        _line.push_back(keystroke);
        _cursor.column();
        _prompt();
        auto matched = _store.matches(_line);
        if (matched){
          _out << color(Green);
        }
        _out << _line;
        if (matched){
          _out << color(Reset);
        }
        break;
      }
    }
  }
  return 0;
}

