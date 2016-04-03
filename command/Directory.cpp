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

  auto cwdRule = x3::rule<class cwd>()
    = x3::lit("$.cwd");
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

  // FIXME: this also resolves symlinks, which we do not want
  target = canonical(target);

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

/// Helper class to expose cwd as an internal command
class Cwd: public Command {
  const Cd& _cd;

public:
  Cwd(const Cd& cd)
  : _cd{cd}
  {
  }

  Command::Status execute(const Line & line, Output & out) override {
    auto iter = line.begin();
    const auto& endIter = line.end();
    bool ok {x3::phrase_parse(iter, endIter, cwdRule, x3::space)};
    if (!ok) return Command::Status::NoMatch;
    out << _cd.cwd().string();

    return Command::Status::Ok;
  }

  bool matches(const Line & /*line*/) const override {
    return false;
  }

  Suggestions suggestions(const Line & /*line*/) const override{
    return Suggestions{};
  }
};

namespace {
  auto& cd = CommandStore::store<Cd>();
  auto& cwd = CommandStore::store<Cwd>(cd);
}
