#ifndef VARIABLE_AST
#define VARIABLE_AST

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <string>

namespace ast {
  namespace x3 = boost::spirit::x3;

  struct sync{};
  struct remove{};
  struct assign{
    std::string value;
  };

  struct list{};

  struct functions: x3::variant<remove, sync, assign>{
    using base_type::base_type;
    using base_type::operator=;
  };

  struct Variable {
    std::string name;
    boost::optional<functions> function;
  };

  struct Variables: x3::variant<Variable, list>{
    using base_type::base_type;
    using base_type::operator=;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::Variable, name, function)
BOOST_FUSION_ADAPT_STRUCT(ast::assign, value)

#endif
