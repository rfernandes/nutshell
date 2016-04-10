#include "Function.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;

struct end{};

namespace {
  namespace x3 = boost::spirit::x3;

  // FIXME: Try to use *char >> attr{done_tag_type}*char to generate pair<string, done_tag_type>
  const auto command = x3::rule<class command, vector<string>>()
    = '{' >> *(~x3::char_('}')) >> -('}' >> x3::attr(string{}));
}

using namespace std;

Command::Status Function::execute(const Line& line, Output& out)
{
  auto iter = line.begin();
  auto endIter = line.end();

  vector<string> data;

  const bool ok {x3::parse(iter, endIter, command, data)};

  if (!ok) {
    return Command::Status::NoMatch;
  }

  if (data.size() == 2) {
    out << "Read function :"<< data.front() << "\n";
    return Command::Status::Ok;
  } else {
    return Command::Status::Incomplete;
  }
}

bool Function::matches(const Line& line) const
{
  auto iter = line.begin();
  auto endIter = line.end();

  const bool ok {x3::parse(iter, endIter, command)};

  return ok || iter != line.begin();
}

Command::Suggestions Function::suggestions(const Line& /*line*/) const
{
  return Suggestions{};
}

namespace {
  auto& registration = CommandStore::store<Function>();
}
