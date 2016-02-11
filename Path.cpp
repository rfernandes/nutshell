#include "Path.h"

#include "Command.h"

namespace fs = std::experimental::filesystem;

Path::Path()
: _path{"/usr/bin", "/home/c/System/bin"}
{
  auto& command = Command::instance();

  for (const auto& dir: _path) {
    for (const auto& entry: fs::directory_iterator{dir}) {
      if (static_cast<bool>(entry.status().permissions() & fs::perms::others_exec)) {
        command.store(entry.path().filename(), Command::Executable{entry.path().string()});
      }
    }
  }
}
