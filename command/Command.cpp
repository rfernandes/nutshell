#include "Command.h"

#include <algorithm>
#include <type_traits>

#include <experimental/string_view>

using namespace std;
using namespace std::experimental;

CommandStore& CommandStore::instance() {
  static CommandStore instance;
  return instance;
}

namespace {
  string_view make_view(string::const_iterator begin,
                        string::const_iterator end){
    return {&*begin, distance(begin, end)};
  }
}

Segment::Segment(Segment::Type type,
                 std::string::const_iterator begin,
                 std::string::const_iterator end,
                 std::string info)
: type{type}
, view{make_view(begin, end)}
, info{info}
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
