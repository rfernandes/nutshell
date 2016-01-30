#include "Command.h"

#include "Shell.h"

Command::Command(Shell &shell)
: _shell(shell)
, _matches{"ls", "exit"}
{
}

void Command::push(unsigned letter) {
  _command.push_back(letter);
  if (_matches.count(_command))
  {
    _shell.event(Shell::Event::COMMAND_MATCHED);
  }
}

void Command::pop() {
  if (!_command.empty()) {
    _command.pop_back();
  }
}

void Command::operator()() {
  if (_command=="exit")
  {
    _shell.event(Shell::Event::SHELL_EXIT);
    return;
  }
  // FIXME: not implemented
  _shell.event(Shell::Event::COMMAND_ERROR_NOT_FOUND);
  _command.clear();
}
