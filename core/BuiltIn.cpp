#include "BuiltIn.h"
#include <string_view>

using namespace std;

bool BuiltIn::matches(const Line& line) const{
  return line.substr(0, line.find_first_of(' ')) == _command;
}

BuiltIn::BuiltIn(string command, string help, Function function)
: _command{move(command)}
, _help{move(help)}
, _function{move(function)}
{
}

BuiltIn::~BuiltIn() = default;

ParseResult BuiltIn::parse(const Line& line, Output& output) {
  // TODO: Rework this as ParseDesc instance
  auto matched = matches(line);

  ParseResult desc;
  if (matched){
    desc.data(line);
    desc.status(Status::Ok);
    desc.segments().emplace_back(Segment::Type::Builtin, line.begin(), line.end(), _help);
  }

  return desc;
}

void BuiltIn::execute(const ParseResult& parseResult, Output& output) {
  const auto& data = any_cast<const Line>(parseResult.data());
  _function(data, output);
}

