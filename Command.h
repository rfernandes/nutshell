#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>

#include <experimental/filesystem>

class Shell;

class Command
{
  Shell &_shell;
  std::string _command;

  using execution = std::function<std::string(const std::string& params)>;

  std::vector<std::experimental::filesystem::path> _path;
  std::unordered_map<std::string, execution> _matches;
  bool _matching;

  bool matches();

public:
  Command(Shell &shell);

  const std::string& get() const { return _command; };

  void push(unsigned letter);
  void pop();

  bool empty() const;
  unsigned width() const;

  std::string operator()();
};

#endif