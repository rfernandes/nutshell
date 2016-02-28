#ifndef PID_H
#define PID_H

#include "Command.h"

class Pid : public Command{
public:
  Command::Status execute(const Line & line, Curses & curses) override;
  bool matches(const Line & line) const override;
  Suggestions suggestions(const Line & line) const override;
};

#endif
