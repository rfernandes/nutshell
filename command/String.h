#ifndef STRING_H
#define STRING_H

#include <command/Parser.h>
#include <command/StringGrammar.h>

struct StringTrait{
  using Rule = parser::string_type;
  using Data = ast::String;

  static const Rule& rule();
};

class String: public parser::RuleCommand<StringTrait>{
public:
  void execute(typename StringTrait::Data& data, Output& output) override;
};

#endif
