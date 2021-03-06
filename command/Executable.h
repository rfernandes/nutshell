#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include <core/Command.h>

#include <set>
#include <experimental/filesystem>

class Executable: public Command {
  const std::set<std::string> _paths;

public:
  explicit Executable(std::set<std::string> paths);

  ParseResult parse(const Line& line, Output& output) override;
  void execute(const ParseResult & parseResult, Output & output) override;
};

#endif
