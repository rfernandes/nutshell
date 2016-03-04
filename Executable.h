#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include "Command.h"

#include <experimental/filesystem>

class Executable: public Command {
  std::experimental::filesystem::path _path;
public:
  Executable(const std::experimental::filesystem::path& path);

  Command::Status execute(const Line& line, Output& out) override;
  bool matches(const Line& line) const override;
  Suggestions suggestions(const Line& line) const override;
};

#endif
