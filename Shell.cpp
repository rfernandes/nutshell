#include "Shell.h"

#include <ncurses.h>

using namespace std;

Shell::Shell()
: _command{*this}
, _directory{_command}
{
  _prompt();
}

void Shell::event(Event event)
{
  _events.push(event);
}

int Shell::run() {
  while (unsigned keystroke = _curses.get()) {
    _curses.refresh();
    switch (keystroke) {
      case '\n':
        _curses << '\n' << _command(_line);
        _history.add(_line);
        _line = Line{};
        event(Event::PROMPT_DISPLAY);
        break;
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
        event(Event::SHELL_EXIT);
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
        auto matched = _command.matches(_line);
        _prompt();
        _curses << color(matched ? 2: 0) << _line.command() << reset;
        if (_line.parameterCount())
          _curses << ' ' <<  _line.parameters();
        break;
      }
    }

    while (!_events.empty()) {
      auto event =_events.front();
      _events.pop();
      switch (event) {
        case Event::COMMAND_ERROR_NOT_FOUND:
          _curses << "NutShell: command not found\n";
          break;
        case Event::PROMPT_DISPLAY:
          _prompt();
          break;
        case Event::SHELL_EXIT:
          goto EXIT;
      }
    }
    refresh();
  }
EXIT:
  return 0;
}

