#ifndef FUNCTION_H
#define FUNCTION_H

#include <command/Command.h>

#include <unordered_map>

class Function: public Command {
  std::unordered_map<std::string, std::string> _functions;
public:
  Function(std::unordered_map<std::string, std::string> functions);

  bool matches(const Line & line) const override;
  Suggestions suggestions(const Line & line) const override;
  Command::Status execute(const Line & line, Output & out) override;
};

#endif
