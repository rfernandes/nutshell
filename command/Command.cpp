#include "Command.h"

#include <algorithm>

using namespace std;

CommandStore& CommandStore::instance() {
  static CommandStore instance;
  return instance;
}

bool CommandStore::matches(const Line& line) const {
  return any_of(_commands.begin(),
                _commands.end(),
                [&](const auto &command){ return command->matches(line); });
}

Command::Suggestions CommandStore::suggestions(const Line& line) const {
  Command::Suggestions ret;
  for (auto &command: _commands){
    const auto commandSuggestions = command->suggestions(line);
    ret.insert(ret.begin(), commandSuggestions.begin(), commandSuggestions.end());
  }
  return ret;
}

Command::Status CommandStore::execute(const Line& line, Output& out) {
  Command::Status ret {Command::Status::NoMatch};
  for (auto &command: _commands) {
    ret = command->execute(line, out);
    if (ret != Command::Status::NoMatch) {
      break;
    }
  }
  return ret;
}
