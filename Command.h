#ifndef COMMAND_H
#define COMMAND_H

#include "Line.h"

#include <unordered_set>
#include <unordered_map>
#include <functional>

#include <experimental/filesystem>

class Shell;

class Command
{
  using Execution = std::function<std::string(const std::string& params)>;

  Shell &_shell;
  std::vector<std::experimental::filesystem::path> _path;
  std::unordered_map<std::string, Execution> _matches;

public:
  Command(Shell &shell);
  std::string operator()(const Line& line);
  bool matches(const Line& line) const;
};

#endif