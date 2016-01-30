#ifndef SHELL_H
#define SHELL_H

#include "Terminal.h"
#include "Prompt.h"
#include "Command.h"

#include <queue>

class Shell
{
public:

  enum class Event {
    COMMAND_MATCHED,
    COMMAND_ERROR_NOT_FOUND,
    PROMPT_DISPLAY,
    SHELL_EXIT
  };

  Shell();

  void event(Event event);

  int run();

private:

  Terminal _term;
  Prompt _prompt;
  Command _command;
  std::queue<Event> _events;
};

#endif