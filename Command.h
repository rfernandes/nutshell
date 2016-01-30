#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <unordered_set>

class Shell;

class Command
{
  Shell &_shell;
  std::string _command;
  std::unordered_set<std::string> _matches;
public:
  Command(Shell &shell);

  const std::string get() const { return _command; };

  void push(unsigned letter);
  void pop();

  void operator()();
};

#endif