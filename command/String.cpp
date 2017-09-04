#include "String.h"

#include <command/StringGrammar.h>
#include <command/VariableGrammar.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;

namespace x3 = boost::spirit::x3;

namespace parser{
  const auto escape_code = [](auto &ctx) {
    char escape;
    switch (char ch = x3::_attr(ctx)) {
      case 'e': escape = '\x1b'; break;
      case 't': escape = '\t'; break;
      case 'n': escape = '\n'; break;
      case 'r': escape = '\r'; break;
      case 'v': escape = '\v'; break;
      case 'a': escape = '\a'; break;
      default:  escape = ch; break;                // Use original letter (escape was not required)
    }
    x3::_val(ctx) = escape;
  };

  const auto escape_code2 = [](auto &ctx) {
    const string asd = x3::_attr(ctx);
    cout << " Replace with value when appropriate >" << asd << "<\n";
  };

  struct escape_class {};
  struct var_class{};

  using escape_type = x3::rule<escape_class, char>;
  using var_type = x3::rule<var_class, string>;

  const escape_type escape = "escape";
  const var_type var = "var";
  const string_type string = "command";

  const auto escape_def = '\\' >> x3::char_[escape_code];
  const auto var_def = x3::lexeme['$' >> (x3::alpha >> *x3::alnum)];

  const auto string_def =
    x3::lexeme['"' >> *(x3::raw[escape] | var | x3::raw[~x3::char_('"')] ) >> '"'] |
    x3::lexeme['\'' >> *~x3::char_('\'') >> '\''];

  BOOST_SPIRIT_DEFINE(
    escape,
    var,
    string
  )
}

const StringTrait::Rule & StringTrait::rule(){
  return parser::string;
}

void String::execute(typename StringTrait::Data& data, Output& output){
  for (const auto &a: data){
    output << a;
  }
}

namespace {
  auto& registration = CommandStore::store<String>();
}

