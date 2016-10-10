#ifndef PID_H
#define PID_H

#include <command/Command.h>

class Pid : public Command{
public:
  ParseResult parse(const Line& line, Output& output, bool execute) override;
};

#endif
