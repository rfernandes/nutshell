#include "Shell.h"
#include "Command.h"

//FIXME: Work towards removing this and relying solely on Curses.h
#include <ncurses.h>

using namespace std;

class BuiltIn: public Command
{
public:
  using Function = function<Command::Status(const Line&, Curses&)>;

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

  Command::Status execute(const Line& line, Curses& curses) override {
    return matches(line) ? _function(line, curses) : Command::Status::NoMatch;
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
                               [&](const Line& /*line*/, Curses& /*curses*/){
                                 _exit = true;
                                 return Command::Status::Ok;
                               });

  CommandStore::store<BuiltIn>("help",
                               [](const Line& /*line*/, Curses& curses){
                                 curses << "No one can help you (for now)\n";
                                 return Command::Status::Ok;
                               });
  _prompt();
}

int Shell::run() {
  unsigned keystroke;
  while (!_exit && (keystroke = _curses.get())) {
    _curses.refresh();
    switch (keystroke) {
      case '\n':
        _curses << '\n';
        try{
          switch (_store.execute(_line, _curses)) {
            case Command::Status::NoMatch:
              _curses << "Command not found [" << _line.command() << "]\n";
              break;
            case Command::Status::Ok:
              break;
          }
        } catch (exception& ex) {
          _curses << "Error " << ex.what() << "\n";
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
            _curses << " " << suggestion;
          }
          _curses << "\n";
        }
        break;
      }
      case 127:
      case KEY_BACKSPACE:
        {
          if (_line.empty()) break;
          unsigned x, y;
          getyx(stdscr, y, x);
          move(y,x-1);
        }
      case KEY_DC:
        _line.pop();
        delch();
        break;
      case KEY_LEFT:
      case KEY_RIGHT:
      {
        unsigned x, y;
        getyx(stdscr, y, x);
        move(y, keystroke == KEY_LEFT ? max(x - 1, _prompt.width() - 1)
                                      : min(x + 1, _prompt.width() - 1 + _line.width()));
        break;
      }
      case KEY_HOME:
      case KEY_END:
      {
        unsigned x, y;
        getyx(stdscr, y, x);
        move(y, keystroke == KEY_HOME ? _prompt.width() - 1
                                      : _prompt.width() - 1 + _line.width());
        break;
      }
      case KEY_DOWN:
      case KEY_UP:
      {
        _line = keystroke == KEY_DOWN ? _history.forward() : _history.backward();
        unsigned x,y;
        getyx(stdscr, y, x);
        deleteln();
        move(y,0);
        _prompt();
        _curses << _line();
        break;
      }
      case 3: //Ctrl-C
      case 18: //Ctrl-R
        _curses << "History interactive mode unimplemented\n";
        break;
      case 4: //Ctrl-D
        _exit = true;
        break;
      case KEY_RESIZE:
        break; // ignore list
      default:
      {
        using namespace curses_manip;
        _line.push(keystroke);
        _curses << keystroke;
        unsigned x,y;
        getyx(stdscr, y, x);
        deleteln();
        move(y,0);
        auto matched = _store.matches(_line);
        _prompt();
        _curses << color(matched ? 2: 0) << _line.command() << reset;
        if (_line.parameterCount())
          _curses << ' ' <<  _line.parameters();
        break;
      }
    }
    refresh();
  }
  return 0;
}

