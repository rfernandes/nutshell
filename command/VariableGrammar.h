#ifndef VARIABLE_GRAMMAR_H
#define VARIABLE_GRAMMAR_H

#include <command/VariableAst.h>

namespace parser{

namespace x3 = boost::spirit::x3;

struct variable_class{};
using variable_type = x3::rule<variable_class, ast::Variables>;
BOOST_SPIRIT_DECLARE(variable_type)

}

#endif
