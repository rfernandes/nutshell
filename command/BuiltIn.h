#ifndef BUILTIN_H
#define BUILTIN_H

#include <command/Command.h>

#include <functional>

class BuiltIn: public Command
{
  bool matches(const Line& line) const;

public:
  using Function = std::function<Status(const Line&, Output&)>;

  BuiltIn(std::string command, std::string help, Function function);
  ~BuiltIn() override;

  ParseResult parse(const Line& line, Output& output, bool execute) override;

private:
  const std::string _command;
  const std::string _help;
  const Function _function;
};


#endif
