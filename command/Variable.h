#ifndef VARIABLE_H
#define VARIABLE_H

#include <command/Command.h>

#include <unordered_map>

class Variable: public Command{
  std::unordered_map<std::string, std::string> _variables;

  friend class VariableVisitor;
public:
  Variable();
  Command::Status execute(const Line& line, Output& out) override;
  bool matches(const Line& line) const override;
  Suggestions suggestions(const Line& line) const override;
};

#endif
