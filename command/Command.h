#ifndef COMMAND_H
#define COMMAND_H

#include <shell/Line.h>
#include <io/Output.h>

#include <experimental/string_view>
#include <unordered_set>
#include <memory>
#include <vector>

class Command;

enum class Status {
  NoMatch,
  Ok,
  Incomplete,       // Command requires additional input
};

std::experimental::string_view slice(const std::string& str, size_t start, size_t length);

struct Segment{
  enum class Type{Command, Builtin, Function, Parameter, Argument, String};
  Type type;
  std::experimental::string_view view;
};

class Description{
  Status _status;
  const Line& _line;
  std::vector<Segment> _segments;

public:
  Description(const Line& line);

  const Line& line() const;

  Status status() const;
  void status(Status status);

  const std::vector<Segment>& segments() const;
  std::vector<Segment>& segments();
};

class Command {
public:

  virtual ~Command() = default;

  virtual Description parse(const Line& line, Output& output, bool execute) = 0;
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

  Description parse(const Line& line, Output& output, bool execute);

  friend class Shell;
  friend class Function;
};

#endif
