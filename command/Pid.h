#ifndef PID_H
#define PID_H

#include <command/Command.h>

class Pid : public Command{
public:
  Command::Status execute(const Line& line, Output& out) override;
  bool matches(const Line& line) const override;
  Suggestions suggestions(const Line& line) const override;
};

#endif
