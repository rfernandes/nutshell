#include "String.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;

namespace {
  namespace x3 = boost::spirit::x3;

  const auto escape_code = [](auto &ctx) {
    char escape;
    switch (x3::_attr(ctx)) {
      case 'e': escape = '\x1b'; break;
      case 't': escape = '\t'; break;
      case 'n': escape = '\n'; break;
      case 'r': escape = '\r'; break;
      case 'v': escape = '\v'; break;
      case 'a': escape = '\a'; break;
      default:  break;                // Use original letter (escape was not required)
    }
    x3::_val(ctx) = escape;
  };

  const auto escape = x3::rule<class escape, char>()
   = '\\' >> x3::char_[escape_code];

  const auto command = x3::rule<class command, string>()
    = ('"' >> x3::no_skip[ * (escape | ~x3::char_('"'))] >> '"') |
      ('\'' >> x3::no_skip[ * (~x3::char_('\''))] >> '\'');
}

Command::Status String::execute(const Line& line, Output& out) {
  auto iter = line.begin();
  auto endIter = line.end();

  string data;

  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  out << data;
  return Command::Status::Ok;
}

bool String::matches(const Line& line) const {
  auto iter = line.begin();
  auto endIter = line.end();

  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space)};

  return ok || static_cast<size_t>(distance(line.begin(), iter)) == line.size();
}

Command::Suggestions String::suggestions(const Line& /*line*/) const
{
  return {};
}

namespace {
  auto& registration = CommandStore::store<String>();
}

