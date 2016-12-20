#ifndef DIRECTORY_GRAMMAR_H
#define DIRECTORY_GRAMMAR_H

#include <command/DirectoryAst.h>
#include <command/Parser.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace parser{
  namespace x3 = boost::spirit::x3;

  struct directory_class {};
  using directory_type = x3::rule<directory_class, ast::CdCommand>;
  BOOST_SPIRIT_DECLARE(directory_type)
}

#endif
