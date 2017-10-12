#ifndef COMMAND_H
#define COMMAND_H

#include <core/Utils.h>
#include <shell/Line.h>
#include <io/Output.h>

#include <any>
#include <memory>
#include <unordered_set>
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
  std::string_view view;
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

  void data(const std::any& data){ _data = data; }
  const std::any& data() const noexcept { return _data; }

private:

  std::any _data;
  Status _status;
  Segments _segments;

};

class Command {
public:

  virtual ~Command() = default;

  virtual ParseResult parse(const Line& line, Output& output) = 0;
  virtual void execute(const ParseResult& parseResult, Output& output) = 0;
};

class CommandStore{
public:

  struct StoreParseResult{
    ParseResult parseResult;
    Command* command;
  };

  template<typename CommandT, typename ...Args>
  static CommandT& store(Args&& ...args) {
    const auto& result = instance()._commands.insert(std::make_unique<CommandT>(std::forward<Args>(args)...));
    return *static_cast<CommandT*>(result.first->get());
  }

private:
  static CommandStore& instance();
  std::unordered_set<std::unique_ptr<Command>> _commands;

  StoreParseResult parse(const Line& line, Output& output);

  friend class Shell;
  friend class Function;
};

#endif
