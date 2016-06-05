#include "Command.h"

#include <algorithm>

using namespace std;

std::experimental::string_view slice(const std::string& str, size_t start, size_t length)
{
  return {str.c_str() + start, length};
}

CommandStore& CommandStore::instance() {
  static CommandStore instance;
  return instance;
}

Description CommandStore::parse(const Line& line, Output& output, bool execute) {
  Description desc;
  for (const auto& command: _commands) {
    desc = command->parse(line, output, execute);
    if (desc.status != Status::NoMatch){
      break;
    }
  }
  return desc;
}

