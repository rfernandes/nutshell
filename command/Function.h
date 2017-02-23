#ifndef FUNCTION_H
#define FUNCTION_H

#include <command/Command.h>

#include <unordered_map>

class Function: public Command {
public:
  using Functions = std::unordered_map<std::string, std::string>;
  explicit Function();

  ParseResult parse(const Line& line, Output& output, bool execute) override;

private:
  Functions _functions;

};

#endif
