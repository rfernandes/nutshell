#include "Command.h"

#include "Shell.h"

using namespace std;

// Pipe to capture output
Command::Status execute(const Line &line, Curses& curses) {
  unique_ptr<FILE, decltype(&pclose)> pipe{popen((line.command() + " " + line.parameters()).data(), "r"), pclose};
  if (!pipe){
    throw std::runtime_error("Unable to fork/pipe");
  }
  constexpr size_t bufferSize{128};
  char buffer[bufferSize];
  string result;
  while (!feof(pipe.get())) {
    if (fgets(buffer, bufferSize, pipe.get()) != NULL)
      curses << buffer;
  }
  return Command::Status::Ok;
}

Command::Command() = default;

Command::Executable::Executable(const std::string& name)
: _name{name}
{
}

Command::Status Command::Executable::operator()(const Line& line, Curses& curses)
{
  return ::execute(line, curses);
}

Command& Command::instance() {
  static Command instance;
  return instance;
}

bool Command::matches(const Line& line) const {
  return _matches.count(line.command());
}

Command::Status Command::execute(const Line& line, Curses& curses) {
  auto command_it(_matches.find(line.command()));
  if (command_it != _matches.end()) {
    command_it->second(line, curses);
    return Status::Ok;
  } else if (!line.command().empty()){
    return Status::CommandNotFound;
  }
  return Status::CommandNotFound; // FIXME: invalid default
}

void Command::store(const std::string& name, Execution execution) {
  _matches.emplace(name, execution);
}
