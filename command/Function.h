#ifndef FUNCTION_H
#define FUNCTION_H

#include <command/Command.h>

class Function: public Command {

public:
  bool matches(const Line & line) const override;
  Suggestions suggestions(const Line & line) const override;
  Command::Status execute(const Line & line, Output & out) override;
};

#endif
