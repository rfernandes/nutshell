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
  Description descDefault{line};
  for (const auto& command: _commands) {
    Description descResult {command->parse(line, output, execute)};
    if (descResult.status() != Status::NoMatch){
      return descResult;
    }
  }
  return descDefault;
}

Description::Description(const Line& line)
: _status{Status::NoMatch}
, _line{line}
{}

const Line& Description::line() const{
  return _line;
}

const std::vector<Segment>& Description::segments() const{
  return _segments;
}

std::vector<Segment>& Description::segments(){
  return _segments;
}

Status Description::status() const{
  return _status;
}

void Description::status(Status status){
  _status = status;
}
