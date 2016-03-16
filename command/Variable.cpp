#include "Variable.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;

namespace ast {
  class sync{};
  class remove{};

  using Name = string;
  using Value = string;

  using functions = boost::variant<remove, sync, Value>;

  struct VariableCommand{
    Name name;
    boost::optional<functions> function;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::VariableCommand, name, function)

namespace {
  namespace x3 = boost::spirit::x3;

  auto sync = x3::rule<class sync, ast::sync>()
    = ".sync" >> x3::attr(ast::sync{}) >> '(' >> ')';

  auto remove = x3::rule<class remove, ast::remove>()
    = ".remove" >> x3::attr(ast::remove{}) >> '(' >> ')';

  auto variableValue = x3::rule<class variableValue, ast::Value>()
    = '"' >> x3::no_skip[+~x3::char_('"')] >> '"' |
      +~x3::char_(' ');

  auto assign = x3::rule<class assign, ast::Value>()
    = '=' >> variableValue;

  auto functions = x3::rule<class functions, ast::functions>()
    = remove | sync | assign;

  auto variableName = x3::rule<class variableName, ast::Name>()
    = x3::no_skip[ x3::alpha >> *x3::alnum ];

  auto command = x3::rule<class command, ast::VariableCommand>()
    = '$' >> variableName >> -functions;
}

class VariableVisitor {
  const string &_name;
  Variable &_variable;
  Output& _out;

  public:
    VariableVisitor(const string& name, Variable &variable, Output& output)
    : _name{name}
    , _variable{variable}
    , _out{output}
    {
    }

    void operator()(const ast::sync&) const {
      _out << "Sync variable\n";
    }

    void operator()(const ast::remove&) const {
      _out << "Removing variable\n";
    }

    void operator()(const ast::Value& value) const {
      _variable._variables[_name] = value;
    }
};

Variable::Variable()
: _variables{{"test", "aaa"}}
{
}

Command::Status Variable::execute(const Line& line, Output& out)
{
  const string input {line()};
  auto iter = input.begin();
  auto endIter = input.end();

  ast::VariableCommand data;
  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  if (data.function) {
    boost::apply_visitor(VariableVisitor{data.name, *this, out}, data.function.get());
  } else {
    out << _variables.at(data.name) << "\n";
  }
  return Command::Status::Ok;
}

bool Variable::matches(const Line& line) const
{
  const string input {line()};
  auto iter = input.begin();
  auto endIter = input.end();

  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space)};

  return ok || static_cast<size_t>(distance(input.begin(), iter)) == input.size();
}

Command::Suggestions Variable::suggestions(const Line& /*line*/) const
{
  return {};
}

namespace {
  auto registration = CommandStore::store<Variable>();
}

