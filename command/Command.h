#ifndef COMMAND_H
#define COMMAND_H

#include <shell/Line.h>
#include <io/Output.h>

#include <unordered_set>
#include <memory>

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
  static CommandT& store(Args&& ...args) {
    const auto& result = instance()._commands.insert(std::make_unique<CommandT>(std::forward<Args>(args)...));
    return *static_cast<CommandT*>(result.first->get());
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
