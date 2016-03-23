#include "Directory.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;
using namespace std::experimental::filesystem;

namespace ast {
  struct previous{};
  struct next{};

  using functions = boost::variant<previous, next, string>;

  struct CdCommand{
    boost::optional<functions> parameters;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::CdCommand, parameters)

namespace {
  namespace x3 = boost::spirit::x3;

  auto parameters = x3::rule<class parameters, ast::functions>()
    = '-' >> x3::attr(ast::previous{}) |
      '+' >> x3::attr(ast::next{})  |
      *x3::char_;      // TODO: create path rule

  auto cdRule = x3::rule<class cdRule, ast::CdCommand>()
    = "cd" >> -parameters;
}

class CdVisitor {
  Cd& cd;

public:
  CdVisitor(Cd& cd): cd(cd){}

  path operator()(const ast::previous&) const {
    if (cd._idx > 0) {
      --cd._idx;
    };
    return cd._history[cd._idx];
  }

  path operator()(const ast::next&) const {
    if (cd._idx < cd._history.size() - 1) {
      ++cd._idx;
    };
    return cd._history[cd._idx];
  }

  path operator()(const std::string& str) const {
    path target{str};
    if (!target.is_absolute()) {
      target = cd.cwd() / target;
    };
    return target;
  }
};

Cd::Cd()
: _current{current_path()}
, _history(1, _current)
, _idx{0}
{
}

bool Cd::matches(const Line & line) const {
  auto iter = line.begin();
  const auto& end = line.end();

  return x3::phrase_parse(iter, end, cdRule, x3::space);
}

Command::Suggestions Cd::suggestions(const Line & /*line*/) const {
  return {};
}

Command::Status Cd::execute(const Line& line, Output& /*out*/) {
  auto iter = line.begin();
  const auto& endIter = line.end();

  ast::CdCommand data;
  bool ok {x3::phrase_parse(iter, endIter, cdRule, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  path target = data.parameters ? boost::apply_visitor(CdVisitor{*this}, data.parameters.get())
                                : path{getpwuid(geteuid())->pw_dir};

  canonical(target);

  error_code error;
  current_path(target, error);

  if (error) { // TODO: use throwing version instead
    throw std::runtime_error("Error while running cd");
  }

  _current = target;
  if (_idx == _history.size()  - 1){
    _idx = _history.size();
    _history.emplace_back(target);
  }

  return Command::Status::Ok;
}

const path& Cd::cwd() const {
  return _current;
}

namespace {
  auto command = CommandStore::store<Cd>();
}
