#ifndef STRING_H
#define STRING_H

#include <command/Command.h>

class String: public Command{
  friend class StringVisitor;
public:
  Command::Status execute(const Line& line, Output& out) override;
  bool matches(const Line& line) const override;
  Suggestions suggestions(const Line& line) const override;
};


#endif
