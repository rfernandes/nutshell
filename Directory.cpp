#include "Directory.h"

#include "Command.h"

#include <unistd.h>
#include <pwd.h>

using namespace std;
using namespace std::experimental::filesystem;

Directory::Directory(Command& command)
: _current{current_path()}
{
  command.store("cd", [&](const std::string& parameters){ return this->cd(parameters); });
}

// Instead of relying on seperate functions leverage extra parameters to support
std::string Directory::cd(std::string parameters) {
  error_code error;

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
  return error ? "Error"
               : (_current = parameters, "");
}

const path& Directory::cd() const {
  return _current;
}
