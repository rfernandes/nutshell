#include "Executable.h"

#include "Curses.h"

using namespace std;

namespace {
  // Pipe to capture output
  Command::Status launch(const Line &line, Curses& curses) {
    unique_ptr<FILE, decltype(&pclose)> pipe{popen((line.command() + " " + line.parameters()).data(), "r"), pclose};
    if (!pipe){
      throw std::runtime_error("Unable to fork/pipe");
    }
    constexpr size_t bufferSize{1024};
    char buffer[bufferSize];
    while (!feof(pipe.get())) {
      if (fgets(buffer, bufferSize, pipe.get()) != NULL) {
        curses << buffer;
      }
    }
    return Command::Status::Ok;
  }
}

Executable::Executable(const std::experimental::filesystem::path& path)
: _path{path}
{
}

bool Executable::matches(const Line& line) const
{
  return line.command() == _path.filename();
}

Command::Suggestions Executable::suggestions(const Line& line) const
{
  const auto& command = line.command();
  const auto& path = _path.filename().string();
  return path.compare(0, command.size(), command) == 0 ? Suggestions{path} : Suggestions{};
}

Command::Status Executable::execute(const Line& line, Curses& curses)
{
  return matches(line) ? launch(line, curses)
                       : Command::Status::NoMatch;
}
