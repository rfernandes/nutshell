#ifndef SHELL_H
#define SHELL_H

#include "Curses.h"
#include "Terminal.h"
#include "Prompt.h"
#include "Command.h"
#include "History.h"
#include "Directory.h"

#include <queue>

class Shell {
public:

  enum class Event {
    SHELL_EXIT,
    PROMPT_DISPLAY,
    COMMAND_ERROR_NOT_FOUND,
  };

  Shell();

  void event(Event event);

  int run();

private:

  Terminal _term;
  Prompt _prompt;
  Command _command;
  Line _line;
  History _history;
  Curses _curses;
  Directory _directory;
  std::queue<Event> _events;
};

#endif