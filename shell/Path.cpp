#include "Path.h"

#include <command/Executable.h>

namespace fs = std::experimental::filesystem;

Path::Path()
: _path{"/bin", "/usr/bin"}
{
  for (const auto& dir: _path) {
    for (const auto& entry: fs::directory_iterator{dir}) {
      if (static_cast<bool>(entry.status().permissions() & fs::perms::others_exec)) {
        CommandStore::store<Executable>(entry.path());
      }
    }
  }
}
