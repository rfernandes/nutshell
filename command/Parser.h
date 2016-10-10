#ifndef PARSER_H
#define PARSER_H

#include <command/Command.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <experimental/string_view>

namespace parser
{
  template<Segment::Type Type>
  struct type_annotation
  {
    template <typename Ast, typename Iterator, typename Context>
    inline void on_success(Iterator const& first,
                           Iterator const& last,
                           Ast& /*ast*/,
                           Context const& context){
      auto& desc = boost::spirit::x3::get<ParseResult>(context).get();
      desc.segments().emplace_back(Type, first, last);
    }
  };
}

#endif
