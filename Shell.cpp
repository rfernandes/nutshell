#include "Shell.h"

#include <ncurses.h>

using namespace std;

Shell::Shell()
: _command(*this){
  _prompt();
  curs_set(1);
  init_pair(2, COLOR_GREEN, -1);
}

void Shell::event(Event event)
{
  _events.push(event);
}

int Shell::run() {
  while (unsigned keystroke = getch())
  {
    refresh();
    switch (keystroke) {
      case '\n':
        addch('\n');
        _history.add(_command.command());
        addstr(_command().data());
        event(Event::PROMPT_DISPLAY);
        break;
      case 127:
      case KEY_BACKSPACE:
        {
          if (_command.empty()) break;
          unsigned x,y;
          getyx(stdscr, y, x);
          move(y,x-1);
        }
      case KEY_DC:
         _command.pop();
         delch();
        break;
      case KEY_LEFT:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y, max(x-1, _prompt.width()) );
        break;
      }
      case KEY_RIGHT:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y, min(x+1, _prompt.width() + _command.width()));
        break;
      }
      case KEY_HOME:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y, _prompt.width());
        break;
      }
      case KEY_END:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y, _prompt.width() + _command.width());
        break;
      }
      case KEY_UP:
      {
        _history.backward();
        break;
      }
      case KEY_DOWN:
      {
        _history.forward();
        break;
      }
      case 3: //Ctrl-C
      case 18: //Ctrl-R
        addstr("History interactive mode unimplemented");
        break;
      case 4: //Ctrl-D
        event(Event::SHELL_EXIT);
        break;
      case KEY_RESIZE:
        break; // ignore list
      default:
        _command.push(keystroke);
        addch(keystroke);
    }

    while (!_events.empty())
    {
      auto event =_events.front();
      _events.pop();
      switch (event)
      {
        case Event::COMMAND_CLEAR_MATCHED:
        case Event::COMMAND_MATCHED:
        {
          unsigned x,y;
          getyx(stdscr, y, x);
          deleteln();
          move(y,0);
          _prompt();
          if (event == Event::COMMAND_MATCHED) {
            attron(A_UNDERLINE);
            attron(COLOR_PAIR(2));
          }
          addstr(_command.command().data());
          if (event == Event::COMMAND_MATCHED) {
            attroff(A_UNDERLINE);
            attroff(COLOR_PAIR(2));
          }
          auto parameters = _command.parameters();
          if (!parameters.empty())
          {
            addch(' ');
          }
          addstr(_command.parameters().data());
          break;
        }
        case Event::COMMAND_ERROR_NOT_FOUND:
          addstr("NutShell: command not found\n");
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

