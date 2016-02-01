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
: _segments(1)
, _active{_segments.begin()}
, _shell{shell}
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
                         [=](const std::string& parameters){
                           return exec(entry.path().string() + " " + parameters);
                         });
      }
    }
  }
}

bool Command::matches() {
  return _matches.count(_segments.front());
}

void Command::push(unsigned letter) {
  switch (letter)
  {
    case ' ':
      if (++_active == _segments.end())
      {
        _segments.emplace_back();
        _active = _segments.begin() + _segments.size() - 1;
      }
      break;
    default:
    {
      _active->push_back(letter);
      _shell.event(matches() ? Shell::Event::COMMAND_MATCHED
                        : Shell::Event::COMMAND_CLEAR_MATCHED);
    }
  }
}

const string& Command::command() const {
  return _segments.front();
}

string Command::parameters() const {
  stringstream ss;
  copy(_segments.begin() + 1, _segments.end(), ostream_iterator<string>(ss, " "));
  string parameters = ss.str();
  if (_segments.size() > 1){
    parameters.resize(parameters.size() - 1);
  }
  return parameters;
}


void Command::pop() {
  if (!_active->empty()) {
    _active->pop_back();
    if (_active == _segments.begin()) {
      _shell.event(matches() ? Shell::Event::COMMAND_MATCHED
                            : Shell::Event::COMMAND_CLEAR_MATCHED);
    }
  } else if (_segments.size() > 1){
    _active = --_segments.erase(_active);
    return;
  }
}

bool Command::empty() const{
  return _active->empty() && _segments.size() == 1;
}

string Command::operator()() {
  string ret;
  //FIXME add assert(!_segments.empty())
  auto command_it(_matches.find(_segments.front()));
  if (command_it != _matches.end()) {
    ret = command_it->second(parameters());
  } else if (!_segments.front().empty()){
    _shell.event(Shell::Event::COMMAND_ERROR_NOT_FOUND);
  }
  _active = _segments.begin();
  _segments.resize(1);
  _active->clear();
  _matching = false;
  return ret;
}

unsigned Command::width() const{
  return accumulate(_segments.begin(),
                    _segments.end(),
                    0u,
                    [](unsigned acc, const string& str){
                      return acc + str.size();
                    }) + _segments.size() - 1;
}