#include "Function.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <iostream>

using namespace std;

namespace {
  namespace x3 = boost::spirit::x3;

  const auto functionName = x3::rule<class functionName, string>()
    = x3::no_skip[ -x3::char_(":") >>  x3::alpha >> *(x3::alnum|x3::char_("_-")) ];

  // FIXME: Try to use *char >> attr{done_tag_type}*char to generate pair<string, done_tag_type>
  const auto command = x3::rule<class command, vector<string>>()
    = functionName >> -('{' >> *(~x3::char_('}')) >> -('}' >> x3::attr(string{})));
}

using namespace std;

Function::Function(unordered_map<string, string> functions)
: _functions{functions}
{}

Command::Status Function::execute(const Line& line, Output& out)
{
  auto iter = line.begin();
  auto endIter = line.end();

  vector<string> data;

  const bool ok {x3::parse(iter, endIter, command, data)};

  if (!ok) {
    return Command::Status::NoMatch;
  }

  switch (data.size()){
    case 3:
      _functions[data.front()] = data[1];
      break;
    case 1: {
      const auto it = _functions.find(data.front());
      if (it == _functions.end()){
        return Command::Status::NoMatch;
      }
      stringstream functionBody{it->second};
      string line;
      while (getline(functionBody, line)) {
        CommandStore::instance().execute(line, out);
      }
      break;
    }
    default:
      return Command::Status::Incomplete;
  }
  return Command::Status::Ok;
}

bool Function::matches(const Line& line) const
{
  auto iter = line.begin();
  auto endIter = line.end();
  vector<string> data;
  const bool ok {x3::parse(iter, endIter, command, data)};

  return (ok || iter != line.begin()) && (!data.empty() && _functions.count(data.front()));
}

Command::Suggestions Function::suggestions(const Line& /*line*/) const
{
  return Suggestions{};
}
