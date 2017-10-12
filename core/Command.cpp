#include "Command.h"

#include <algorithm>
#include <type_traits>
#include <string_view>

using namespace std;

CommandStore& CommandStore::instance() {
  static CommandStore instance;
  return instance;
}

namespace {
  string_view make_view(string::const_iterator begin,
                        string::const_iterator end){
    return {&*begin, static_cast<size_t>(distance(begin, end))};
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

CommandStore::StoreParseResult CommandStore::parse(const Line& line, Output& output) {
  for (const auto& command: _commands) {
    ParseResult descResult {command->parse(line, output)};
    if (descResult.status() != Status::NoMatch){
      return {descResult, command.get()};
    }
  }
  return {ParseResult{}, nullptr};
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
