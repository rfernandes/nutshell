#include "Path.h"

#include <command/Executable.h>

#include <set>

using namespace std;
namespace fs = std::experimental::filesystem;

Path::Path()
: _path{"/bin", "/usr/bin"}
{
  set<string> cache;
  for (const auto& dir: _path) {
    for (const auto& path: fs::directory_iterator{dir}) {
      if (static_cast<bool>(path.status().permissions() & fs::perms::others_exec)) {
        cache.emplace(path.path().filename().string());
      }
    }
  }
  CommandStore::store<Executable>(std::move(cache));
}
