#include "Command.h"

#include "Shell.h"

#include <experimental/filesystem>

#include <iostream>
#include <iterator>
#include <algorithm>
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
{
  for (const auto& dir: _path) {
    for (const auto& entry: fs::directory_iterator{dir}) {
      if (static_cast<bool>(entry.status().permissions() & fs::perms::others_exec)) {
        store(entry.path().filename(),
              [=](const std::string& parameters){
                return exec(entry.path().string() + " " + parameters);
              });
      }
    }
  }
}

bool Command::matches(const Line& line) const {
  return _matches.count(line.command());
}

string Command::operator()(const Line& line) {
  string ret;
  //FIXME add assert(!line.parameters.empty())
  auto command_it(_matches.find(line.command()));
  if (command_it != _matches.end()) {
    ret = command_it->second(line.parameters());
  } else if (!line.command().empty()){
    _shell.event(Shell::Event::COMMAND_ERROR_NOT_FOUND);
  }
  return ret;
}

void Command::store(const std::string& name, Execution execution) {
  _matches.emplace(name, execution);
}
