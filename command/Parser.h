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

  template<typename RuleTrait>
  class RuleCommand: public Command{
  protected:
    virtual void execute(typename RuleTrait::Data& data, Output& output) = 0;

  public:
    ParseResult parse(const Line& line, Output& output, bool exec) override{
      namespace x3 = boost::spirit::x3;
      auto iter = line.begin();
      const auto &endIter = line.end();

      ParseResult desc;
      const auto parser = x3::with<ParseResult>(boost::ref(desc))[RuleTrait::rule()];

      typename RuleTrait::Data data;
      const bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};

      if (ok){
        desc.status(Status::Ok);
        if (exec){
          execute(data, output);
        }
      }

      return desc;
    }
  };
}

#endif
