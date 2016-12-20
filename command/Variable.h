#ifndef VARIABLE_H
#define VARIABLE_H

#include <command/Parser.h>
#include <command/VariableGrammar.h>

#include <boost/spirit/home/x3.hpp>

#include <unordered_map>

struct VariableTrait{
  using Rule = parser::variable_type;
  using Data = ast::Variables;

  static const Rule& rule();
};

class Variable: public parser::RuleCommand<VariableTrait>{

  void execute(typename VariableTrait::Data& data, Output& output) override;
public:
  using Store = std::unordered_map<std::string, std::string>;

  Variable();
  ~Variable() override;

private:
  friend class VariableVisitor;
  Store  _variables;
};

#endif
