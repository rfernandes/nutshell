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


Description CommandStore::parse(const Line& line, Output& output, bool execute) {
  Description descDefault;
  for (const auto& command: _commands) {
    Description descResult {command->parse(line, output, execute)};
    if (descResult.status() != Status::NoMatch){
      return descResult;
    }
  }
  return descDefault;
}

Description::Description()
: _status{Status::NoMatch}
{}

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
