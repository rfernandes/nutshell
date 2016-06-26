#ifndef STRING_H
#define STRING_H

#include <command/Command.h>

class String: public Command{
  friend class StringVisitor;

public:
  Description parse(const Line& line, Output& output, bool execute) override;
};

#endif
