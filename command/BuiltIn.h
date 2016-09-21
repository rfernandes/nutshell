#ifndef BUILTIN_H
#define BUILTIN_H

#include <command/Command.h>

#include <functional>

class BuiltIn: public Command
{
  bool matches(const Line& line) const;

public:
  using Function = std::function<Status(const Line&, Output&)>;

  BuiltIn(std::string command, Function function);
  ~BuiltIn() override;

  Description parse(const Line& line, Output& output, bool execute) override;

private:
  const std::string _command;
  const Function _function;
};


#endif
