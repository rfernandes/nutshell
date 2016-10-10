#ifndef VARIABLE_H
#define VARIABLE_H

#include <command/Command.h>

#include <unordered_map>

class Variable: public Command{
public:
  using Store = std::unordered_map<std::string, std::string>;

  Variable();
  ParseResult parse(const Line& line, Output& output, bool execute) override;

private:
  friend class VariableVisitor;
  Store  _variables;
};

#endif
