#include "String.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;
using namespace std::experimental;

namespace x3 = boost::spirit::x3;

namespace {
  const auto escape_code = [](auto &ctx) {
    char escape;
    switch (x3::_attr(ctx)) {
      case 'e': escape = '\x1b'; break;
      case 't': escape = '\t'; break;
      case 'n': escape = '\n'; break;
      case 'r': escape = '\r'; break;
      case 'v': escape = '\v'; break;
      case 'a': escape = '\a'; break;
      default:  return;                // Use original letter (escape was not required)
    }
    x3::_val(ctx) = escape;
  };

  struct escape_class {};
  struct command_class: parser::type_annotation<Segment::Type::String>{};

  using escape_type = x3::rule<escape_class, char>;
  using command_type = x3::rule<command_class, string>;

  const escape_type escape = "escape";
  const command_type command = "command";

  const auto escape_def = '\\' >> x3::char_[escape_code];
  const auto command_def =
    ('"' >> x3::no_skip[ * (escape | ~x3::char_('"'))] >> '"') |
    ('\'' >> x3::no_skip[ * (~x3::char_('\''))] >> '\'');

  BOOST_SPIRIT_DEFINE(
    escape,
    command
  )
}

Description String::parse(const Line& line, Output& output, bool execute){
  auto iter = line.begin();
  auto endIter = line.end();

  Description desc;
  const auto parser = x3::with<Description>(ref(desc))[command];

  string data;
  const bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};

  if (ok) {
    desc.status(Status::Ok);
    if (execute) {
      output << data;
    }
  }
  return desc;
}

namespace {
  auto& registration = CommandStore::store<String>();
}

