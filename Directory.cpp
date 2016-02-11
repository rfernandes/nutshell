#include "Directory.h"

#include "Command.h"

#include <unistd.h>
#include <pwd.h>

using namespace std;
using namespace experimental::filesystem;

Directory::Directory()
: _current{current_path()}
{
  using namespace placeholders;
  Command::instance().store("cd", [&](const Line& line, Curses& curses){ return this->cd(line, curses); });
}

// Instead of relying on seperate functions leverage extra parameters to support
//  directory stack
Command::Status Directory::cd(const Line& line, Curses& /*curses*/) {
  error_code error;

  string parameters{line.parameters()};

  if (parameters.empty())
  {
    auto passwd = getpwuid(geteuid());
    parameters = passwd->pw_dir;
  }

  path target{parameters};
  if (!target.is_absolute()) {
    target = _current / target;
  }
  current_path(target, error);

  if (error){
    throw std::runtime_error("Error while running cd");
  }
  _current = parameters;
  return Command::Status::Ok;
}

const path& Directory::cd() const {
  return _current;
}
