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
  using Segments = std::vector<std::string>;
  using Execution = std::function<std::string(const std::string& params)>;

  Segments _segments;
  Segments::iterator _active;

  Shell &_shell;


  std::vector<std::experimental::filesystem::path> _path;
  std::unordered_map<std::string, Execution> _matches;
  bool _matching;

  bool matches();

public:
  Command(Shell &shell);

  const std::string& command() const;
  std::string parameters() const;

  void push(unsigned letter);
  void pop();

  bool empty() const;
  unsigned width() const;

  std::string operator()();
};

#endif