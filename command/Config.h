#ifndef CONFIG_H
#define CONFIG_H

#include <boost/spirit/home/x3.hpp>

namespace parser{
  namespace x3 = boost::spirit::x3;
  using iterator_type = std::string::const_iterator;
  using phrase_context_type = x3::phrase_parse_context<x3::space_type>::type;
  using context_type = x3::context<
    ParseResult,
    std::reference_wrapper<ParseResult> const,
    phrase_context_type>;
}

#endif
