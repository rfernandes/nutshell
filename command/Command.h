#ifndef COMMAND_H
#define COMMAND_H

#include <shell/Line.h>
#include <shell/Utils.h>
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

struct Segment{
  enum class Type{Unknown, Command, Builtin, Function, Parameter, Argument, String};
  Type type;
  std::experimental::string_view view;
  std::string info;

  Segment(Type type,
          std::string::const_iterator begin,
          std::string::const_iterator end,
          std::string info = "");
};

template<>
constexpr const char* enum_data<Segment::Type>[] {
  "Unknown", "Command", "Builtin", "Function", "Parameter", "Argument", "String"
};

class ParseResult{
public:
  using Segments = std::vector<Segment>;

  ParseResult();

  Status status() const;
  void status(Status status);

  const Segments& segments() const;
  Segments& segments();

private:
  Status _status;
  Segments _segments;

};

class Command {
public:

  virtual ~Command() = default;

  virtual ParseResult parse(const Line& line, Output& output, bool execute) = 0;
};

class CommandStore : public Command{
public:

  template<typename CommandT, typename ...Args>
  static CommandT& store(Args&& ...args) {
    const auto& result = instance()._commands.insert(std::make_unique<CommandT>(std::forward<Args>(args)...));
    return *static_cast<CommandT*>(result.first->get());
  }

private:
  static CommandStore& instance();
  std::unordered_set<std::unique_ptr<Command>> _commands;

  ParseResult parse(const Line& line, Output& output, bool execute);

  friend class Shell;
  friend class Function;
};

#endif
