#ifndef COMMAND_H
#define COMMAND_H

#include "Line.h"

#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <memory>

class Output;

class Command {
public:
  using Suggestions = std::vector<std::string>;

  virtual ~Command() = default;

  enum class Status {
    NoMatch,
    Ok,
  };

  virtual Status execute(const Line& line, Output& out) = 0;
  virtual bool matches(const Line& line) const = 0;
  virtual Suggestions suggestions(const Line& line) const = 0;
};

class CommandStore {
public:

  template<typename CommandT, typename ...Args>
  bool static store(Args ...args) {
    instance()._commands.insert(std::make_unique<CommandT>(args...));
    return true;
  }

private:
  static CommandStore& instance();
  std::unordered_set<std::unique_ptr<Command>> _commands;

  bool matches(const Line& line) const;
  Command::Suggestions suggestions(const Line& line) const;
  Command::Status execute(const Line& line, Output& output);

  friend class Shell;
};

#endif
