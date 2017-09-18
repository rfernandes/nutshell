#ifndef PARSER_H
#define PARSER_H

#include <core/Command.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

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
    //TODO: replace this function with the parent execute function, collapse this class
    virtual void execute(const typename RuleTrait::Data& data, Output& output) = 0;

  public:
    ParseResult parse(const Line& line, Output& output) override{
      namespace x3 = boost::spirit::x3;
      auto iter = line.begin();
      const auto &endIter = line.end();

      ParseResult desc;
      const auto parser = x3::with<ParseResult>(boost::ref(desc))[RuleTrait::rule()];

      typename RuleTrait::Data data;
      const bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};
      desc.data(data);

      if (ok){
        desc.status(Status::Ok);
//         if (exec){
//           execute(data, output);
//         }
      }

      return desc;
    }

    void execute(const ParseResult& parseResult, Output& output) override{
      const auto& data = std::any_cast<typename RuleTrait::Data>(parseResult.data());
      execute(data, output);
    }
  };
}

#endif
