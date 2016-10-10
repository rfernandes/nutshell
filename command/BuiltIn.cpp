#include "BuiltIn.h"
#include <experimental/string_view>

using namespace std;
using namespace std::experimental;

bool BuiltIn::matches(const Line& line) const{
  return line.substr(0, line.find_first_of(' ')) == _command;
}

BuiltIn::BuiltIn(string command, Function function)
: _command{move(command)}
, _function{move(function)}
{
}

BuiltIn::~BuiltIn() = default;

ParseResult BuiltIn::parse(const Line& line, Output& output, bool execute) {
  auto matched = matches(line);
  if (matched && execute){
    _function(line, output);
  }

  ParseResult desc;
  if (matched){
    desc.status(Status::Ok);
    desc.segments().emplace_back(Segment::Type::Builtin, line.begin(), line.end());
  }

  return desc;
}

