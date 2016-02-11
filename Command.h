#ifndef COMMAND_H
#define COMMAND_H

#include "Line.h"

#include <unordered_set>
#include <unordered_map>
#include <functional>

class Curses;

class Command {
  Command();
public:

  static Command& instance();

  enum class Status {
    Ok,
    CommandNotFound,
  };
  using Execution = std::function<Status(const Line& line, Curses& curses)>;

  Status execute(const Line& line, Curses& curses);
  bool matches(const Line& line) const;

  class Executable {
    const std::string _name;
  public:
    Executable(const std::string& name);

    Status operator()(const Line& line, Curses& curses);
  };

  void store(const std::string& name, Execution execution);

public:
  std::unordered_map<std::string, Execution> _matches;
};

#endif