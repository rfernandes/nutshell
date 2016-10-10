#include "Command.h"

#include <algorithm>

using namespace std;

CommandStore& CommandStore::instance() {
  static CommandStore instance;
  return instance;
}

Segment::Segment(Segment::Type type,
                 std::string::const_iterator begin,
                 std::string::const_iterator end)
: type{type}
, begin{begin}
, end{end}
{
}


ParseResult CommandStore::parse(const Line& line, Output& output, bool execute) {
  ParseResult descDefault;
  for (const auto& command: _commands) {
    ParseResult descResult {command->parse(line, output, execute)};
    if (descResult.status() != Status::NoMatch){
      return descResult;
    }
  }
  return descDefault;
}

ParseResult::ParseResult()
: _status{Status::NoMatch}
{}

const std::vector<Segment>& ParseResult::segments() const{
  return _segments;
}

std::vector<Segment>& ParseResult::segments(){
  return _segments;
}

Status ParseResult::status() const{
  return _status;
}

void ParseResult::status(Status status){
  _status = status;
}
