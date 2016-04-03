#include "Variable.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;

namespace ast {
  struct sync{};
  struct remove{};
  struct assign{
    string value;
  };

  class list{};

  using functions = boost::variant<remove, sync, assign>;

  struct Variable{
    string name;
    boost::optional<functions> function;
  };

  struct Variables{
    boost::variant<list, Variable> command;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::Variable, name, function)
BOOST_FUSION_ADAPT_STRUCT(ast::Variables, command)
BOOST_FUSION_ADAPT_STRUCT(ast::assign, value)

namespace {
  namespace x3 = boost::spirit::x3;

  auto sync = x3::rule<class sync, ast::sync>()
    = ".sync" >> x3::attr(ast::sync{}) >> '(' >> ')';

  auto remove = x3::rule<class remove, ast::remove>()
    = ".remove" >> x3::attr(ast::remove{}) >> '(' >> ')';

  auto list = x3::rule<class list, ast::list>()
    = ".list" >> x3::attr(ast::list{}) >> '(' >> ')';

  auto variableValue = x3::rule<class variableValue, string>()
    = '"' >> x3::no_skip[+~x3::char_('"')] >> '"' |
      +~x3::char_(' ');

  auto assign = x3::rule<class assign, ast::assign>()
    = '=' >> variableValue;

  auto functions = x3::rule<class functions, ast::functions>()
    = remove | sync | assign;

  auto variableName = x3::rule<class variableName, string>()
    = x3::no_skip[ x3::alpha >> *x3::alnum ];

  auto command = x3::rule<class command, ast::Variables>()
    = '$' >> (variableName >> -functions |
              list);
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
      _variable._variables.erase(_name);
    }

    void operator()(const ast::assign& assign) const {
      _variable._variables[_name] = assign.value;
    }
};

Variable::Variable()
: _variables{{"test", "aaa"}}
{
}

Command::Status Variable::execute(const Line& line, Output& out) {
  auto iter = line.begin();
  auto endIter = line.end();

  ast::Variables data;
  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  if (get<ast::Variable>(&data.command)) {
    const auto& variableCommand = get<ast::Variable>(data.command);
    if (variableCommand.function) {
      boost::apply_visitor(VariableVisitor{variableCommand.name, *this, out}, variableCommand.function.get());
    } else {
      out << _variables.at(variableCommand.name);
    }
  } else {
    for (const auto &variable: _variables) {
      out << '$' << variable.first << "=" << variable.second << "\n";
    }
  }
  return Command::Status::Ok;
}

bool Variable::matches(const Line& line) const {
  auto iter = line.begin();
  auto endIter = line.end();

  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space)};

  return ok || static_cast<size_t>(distance(line.begin(), iter)) == line.size();
}

Command::Suggestions Variable::suggestions(const Line& /*line*/) const
{
  return {};
}

namespace {
  auto& registration = CommandStore::store<Variable>();
}

