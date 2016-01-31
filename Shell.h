#ifndef SHELL_H
#define SHELL_H

#include "Terminal.h"
#include "Prompt.h"
#include "Command.h"
#include "History.h"

#include <queue>

class Shell
{
public:

  enum class Event {
    SHELL_EXIT,
    PROMPT_DISPLAY,
    COMMAND_ERROR_NOT_FOUND,
    COMMAND_MATCHED,
    COMMAND_CLEAR_MATCHED,
  };

  Shell();

  void event(Event event);

  int run();

private:

  Terminal _term;
  Prompt _prompt;
  Command _command;
  History _history;
  std::queue<Event> _events;
};

#endif