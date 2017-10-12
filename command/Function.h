#ifndef FUNCTION_H
#define FUNCTION_H

#include <core/Command.h>

#include <unordered_map>

class Function: public Command {
public:
  using Functions = std::unordered_map<std::string, std::string>;
  explicit Function();

  ParseResult parse(const Line& line, Output& output) override;
  void execute(const ParseResult& parseResult, Output& output) override;

private:
  Functions _functions;

};

#endif
