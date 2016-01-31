#include "Command.h"

#include "Shell.h"

#include <experimental/filesystem>

#include <memory>

using namespace std;
namespace fs = std::experimental::filesystem;

string exec(const std::string& cmd) {
  unique_ptr<FILE, decltype(&pclose)> pipe{popen(cmd.data(), "r"), pclose};
  if (!pipe) return "ERROR";
  char buffer[128];
  string result;
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}

namespace builtins {
  static auto help = [](const std::string& /*params*/){
    return "No one can help you (for now)\n";
  };
}

Command::Command(Shell &shell)
: _shell{shell}
, _path{"/usr/bin", "/home/c/System/bin"}
, _matches{
    {"help", builtins::help},
    {"exit", [&](const std::string& /*params*/){
      _shell.event(Shell::Event::SHELL_EXIT);
      return "";
    }}
  }
, _matching{false}
{
  for (const auto& dir: _path) {
    for (const auto& entry: fs::directory_iterator{dir}) {
      if (static_cast<bool>(entry.status().permissions() & fs::perms::others_exec)) {
        _matches.emplace(entry.path().filename(),
                         [=](const std::string& /*parameters*/){
                           return exec(entry.path().string());
                         });
      }
    }
  }
}

bool Command::matches()
{
  return _matches.count(_command);
}

void Command::push(unsigned letter) {
  _command.push_back(letter);

  _shell.event(matches() ? Shell::Event::COMMAND_MATCHED
                         : Shell::Event::COMMAND_CLEAR_MATCHED);
}

void Command::pop() {
  if (!_command.empty()) {
    _command.pop_back();

    _shell.event(matches() ? Shell::Event::COMMAND_MATCHED
                           : Shell::Event::COMMAND_CLEAR_MATCHED);
  }
}

bool Command::empty() const{
  return _command.empty();
}

string Command::operator()() {
  string ret;
  auto command_it(_matches.find(_command));
  if (command_it != _matches.end()) {
    ret = command_it->second(string{});
  } else if (!_command.empty()){
    _shell.event(Shell::Event::COMMAND_ERROR_NOT_FOUND);
  }
  _command.clear();
  _matching = false;
  return ret;
}

unsigned Command::width() const{
  return _command.size();
}