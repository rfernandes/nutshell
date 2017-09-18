#ifndef PID_H
#define PID_H

#include <core/Command.h>

class Pid : public Command{
public:
  ParseResult parse(const Line& line, Output& output) override;
  void execute(const ParseResult& parseResult, Output & output) override;
};

#endif
