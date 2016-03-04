#include "Shell.h"
#include "Command.h"

//FIXME: Work towards removing this and relying solely on Curses.h
#include <ncurses.h>

using namespace std;

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
    return line.command() == _command;
  }

  Suggestions suggestions(const Line& line) const override {
    const auto& command = line.command();
    return _command.compare(0, command.size(), command) == 0 ? Suggestions{_command} : Suggestions{};
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
, _exit{false}
{
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
  unsigned keystroke;
  while (!_exit && (keystroke = _out.get())) {
    _out.refresh();
    switch (keystroke) {
      case '\n':
        _out << '\n';
        try{
          switch (_store.execute(_line, _out)) {
            case Command::Status::NoMatch:
              _out << "Command not found [" << _line.command() << "]\n";
              break;
            case Command::Status::Ok:
              break;
          }
        } catch (exception& ex) {
          _out << "Error " << ex.what() << "\n";
        }
        _history.add(_line);
        _line = Line{};
        _prompt();
        break;
      case   9: { // Tab
        const auto& suggestions = _store.suggestions(_line);
        if (!suggestions.empty()) {
          if (suggestions.size() == 1)
          {
            _line = {suggestions.at(0)};
          }
          for (auto& suggestion: suggestions) {
            _out << " " << suggestion;
          }
          _out << "\n";
        }
        break;
      }
      case 127:
      case KEY_BACKSPACE:
        {
          if (_line.empty()) break;
          _cursor.left(1);
        }
      case KEY_DC:
        _line.pop();
        break;
      case KEY_LEFT:
        if (_cursor.position().x > _prompt.width() - 1)
          _cursor.left();
        break;
      case KEY_RIGHT:
        if (_cursor.position().x < _prompt.width() - 1 + _line.width())
          _cursor.right();
        break;
      case KEY_HOME:
      case KEY_END:
      {
        auto pos = _cursor.position();
        pos.x = keystroke == KEY_HOME ? _prompt.width() - 1
                                      : _prompt.width() - 1 + _line.width();
        _cursor.position(pos);
        break;
      }
      case KEY_DOWN:
      case KEY_UP:
      {
        _line = keystroke == KEY_DOWN ? _history.forward() : _history.backward();
        auto pos = _cursor.position();
        pos.x = 0;
        _cursor.position(pos);
        _prompt();
        _out << _line();
        break;
      }
      case 3: //Ctrl-C
      case 18: //Ctrl-R
        _out << "History interactive mode unimplemented\n";
        break;
      case 4: //Ctrl-D
        _exit = true;
        break;
      case KEY_RESIZE:
        break; // ignore list
      default:
      {
        using namespace manip;
        _line.push(keystroke);
        auto pos = _cursor.position();
        pos.x = 0;
        _cursor.position(pos);
        auto matched = _store.matches(_line);
        _prompt();
        _out << color(matched ? 2: 0) << _line.command() << reset;
        if (_line.parameterCount())
          _out << ' ' <<  _line.parameters();
        break;
      }
    }
    _out.refresh();
  }
  return 0;
}

