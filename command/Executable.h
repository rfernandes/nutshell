#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include <command/Command.h>

#include <set>
#include <experimental/filesystem>

class Executable: public Command {
  const std::set<std::string> _paths;

public:
  Executable(std::set<std::string> paths);

  Description parse(const Line& line, Output& output, bool execute) override;
};

#endif
