#include "Variable.h"
#include "VariableGrammar.h"
#include "Config.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include "Config.h"

using namespace std;
using namespace std::experimental;

namespace x3 = boost::spirit::x3;

// The generic parser lost the ability add a semantic action to check if variable exists
//     auto matcher = [&](auto &ctx){
//       return match_name(ctx, _variables);
//     };

namespace {
  const auto match_name = [](auto &ctx, Variable::Store& variables) {
    const ast::Variables& data = x3::_attr(ctx);
    const auto* variable = boost::get<ast::Variable>(&data);
    if (variable &&
        !variable->function &&
        !variables.count(variable->name)){
      x3::_pass(ctx) = false;
    }
  };
}


namespace parser {

  struct sync_class: parser::type_annotation<Segment::Type::Function> {};
  struct remove_class: parser::type_annotation<Segment::Type::Function> {};
  struct list_class: parser::type_annotation<Segment::Type::Function> {};
  struct variableValue_class: parser::type_annotation<Segment::Type::String> {};
  struct assign_class;
  struct functions_class {};
  struct variableName_class: parser::type_annotation<Segment::Type::Command> {};
  struct sigil_class: parser::type_annotation<Segment::Type::Builtin>{};

  using sync_type = x3::rule<sync_class, ast::sync>;
  using remove_type = x3::rule<remove_class, ast::remove>;
  using list_type = x3::rule<list_class, ast::list>;
  using variableValue_type = x3::rule<variableValue_class, string>;
  using assign_type = x3::rule<assign_class, ast::assign>;
  using functions_type = x3::rule<functions_class, ast::functions>;
  using variableName_type = x3::rule<variableName_class, string>;
  using sigil_type = x3::rule<sigil_class>;

  const sync_type sync = "sync";
  const remove_type remove = "remove";
  const list_type list = "list";
  const variableValue_type variableValue = "variableValue";
  const assign_type assign = "assign";
  const functions_type functions = "functions";
  const variableName_type variableName = "variableName";
  const sigil_type sigil = "sigil";
  const variable_type variable = "variable";

  auto sync_def = ".sync" >> x3::attr(ast::sync{});
  auto remove_def = ".remove" >> x3::attr(ast::remove{});
  auto list_def = ".list" >> x3::attr(ast::list{});
  auto variableValue_def = '"' >> x3::no_skip[+~x3::char_('"')] >> '"' | +~x3::char_(' ');
  auto assign_def = '=' >> variableValue;
  auto functions_def = remove | sync | assign;
  auto variableName_def = x3::no_skip[ x3::alpha >> *x3::alnum ];
  auto sigil_def = '$';
  auto variable_def = sigil >> ((variableName >> -functions) | list);

  BOOST_SPIRIT_DEFINE(
    sync,
    remove,
    list,
    variableValue,
    assign,
    functions,
    variableName,
    sigil,
    variable
  )

  BOOST_SPIRIT_INSTANTIATE(variable_type, iterator_type, context_type)
}


class VariableVisitor {
  Variable &_variable;
  Output& _out;
  string _name;

public:
  VariableVisitor(Variable &variable, Output& output)
  : _variable{variable}
  , _out{output}
  {
  }

  void operator()(const ast::sync&) const {
    _out << "Sync variable\n";
  }

  void operator()(const ast::remove&) const {
    _variable._variables.erase(_name);
  }

  void operator()(const ast::assign& assign) const {
    _variable._variables[_name] = assign.value;
  }

  void operator()(const ast::Variable& variable) {
    if (variable.function) {
      _name = variable.name;
      boost::apply_visitor(*this, variable.function.get());
    } else {
      _out << _variable._variables.at(variable.name);
    }
  }

  void operator()(const ast::list&) const {
    for (const auto &variable: _variable._variables) {
      _out << '$' << variable.first << "=" << variable.second << "\n";
    }
  }
};

Variable::Variable()
: _variables{{"test", "aaa"}}
{
}

Variable::~Variable() = default;

const VariableTrait::Rule& VariableTrait::rule(){
  return parser::variable;
}

void Variable::execute(typename VariableTrait::Data& data, Output& output){
  VariableVisitor visitor{*this, output};
  boost::apply_visitor(visitor, data);
}

namespace {
  auto& registration = CommandStore::store<Variable>();
}

