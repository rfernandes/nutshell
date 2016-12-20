#ifndef DIRECTORY_AST
#define DIRECTORY_AST

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <string>

namespace ast {
  namespace x3 = boost::spirit::x3;

  using path = std::string;

  struct previous{};
  struct next{};

  struct functions: x3::variant<previous, next, std::string>{
    using base_type::base_type;
    using base_type::operator=;
  };

  struct CdCommand{
    boost::optional<functions> parameters;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::CdCommand, parameters)

#endif

