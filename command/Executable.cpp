#include "Executable.h"

using namespace std;

namespace {
  // Pipe to capture output
  Command::Status launch(const Line &line, Output& out) {
    unique_ptr<FILE, decltype(&pclose)> pipe{popen(line.data(), "r"), pclose};
    if (!pipe){
      throw std::runtime_error("Unable to fork/pipe");
    }
    constexpr size_t bufferSize{1024};
    char buffer[bufferSize];
    while (!feof(pipe.get())) {
      if (fgets(buffer, bufferSize, pipe.get()) != NULL) {
        out << buffer;
      }
    }
    return Command::Status::Ok;
  }
}

Executable::Executable(const std::experimental::filesystem::path& path)
: _path{path}
{
}

bool Executable::matches(const Line& line) const {
  const auto& filename = _path.filename().string();
  return filename == line.substr(0, line.find_first_of(' '));
}

Command::Suggestions Executable::suggestions(const Line& line) const {
  return matches(line) ? Suggestions{_path} : Suggestions{};
}

Command::Status Executable::execute(const Line& line, Output& out) {
  return matches(line) ? launch(line, out)
                       : Command::Status::NoMatch;
}
