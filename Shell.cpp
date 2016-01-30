#include "Shell.h"

#include <ncurses.h>

Shell::Shell()
: _command(*this){
  _prompt();
  curs_set(2);
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
        _command();
        event(Event::PROMPT_DISPLAY);
        break;
      case 127:
      case KEY_BACKSPACE:
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y,x-1);
      case KEY_DC:
      {

        delch();
        _command.pop();
        break;
      }
      case KEY_LEFT:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y,x-1);
        break;
      }
      case KEY_RIGHT:
      {
        unsigned x,y;
        getyx(stdscr, y, x);
        move(y,x+1);
        break;
      }
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
        case Event::COMMAND_MATCHED:
          deleteln();
          unsigned x,y;
          getyx(stdscr, y, x);
          move(y,0);
          _prompt();
          attron(COLOR_PAIR(2));
          addstr(_command.get().data());
          attroff(COLOR_PAIR(2));
          break;
        case Event::COMMAND_ERROR_NOT_FOUND:
          addstr("\nNutShell: command not found\n");
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

