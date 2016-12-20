#ifndef STRING_GRAMMAR_H
#define STRING_GRAMMAR_H

#include <command/StringAst.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace parser{
  namespace x3 = boost::spirit::x3;

  struct string_class: parser::type_annotation<Segment::Type::String>{};
  using string_type = x3::rule<string_class, ast::String>;
  BOOST_SPIRIT_DECLARE(string_type)
}

#endif
