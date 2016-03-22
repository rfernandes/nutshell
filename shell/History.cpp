#include "History.h"

#include <command/Command.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;

namespace ast {
  class list{};
  class clear{};

  using functions = boost::variant<list, clear>;

  using HistoryCommand = boost::optional<boost::variant<list, clear>>;
}

namespace {
  namespace x3 = boost::spirit::x3;

  auto list = x3::rule<class list, ast::list>()
    = x3::lit("list") >> x3::attr(ast::list{});

  auto clear = x3::rule<class clear, ast::clear>()
    = x3::lit("clear") >> x3::attr(ast::clear{});

  auto functions = x3::rule<class functions, ast::functions>()
    = clear | list ;

  auto historyCommand = x3::rule<class historyCommand, ast::HistoryCommand>()
    = '!' >> - x3::no_skip[ '.' >> functions ];

  Line empty;
}

class Visitor {
  History& _history;
  Output& _out;

  public:
    Visitor(History& history, Output& output)
    : _history{history}
    , _out{output}
    {
    }

    void operator()(const ast::list&) const {
      auto &list = _history.list();
      for (size_t idx=0; idx< list.size(); ++idx){
        _out << idx << ':' << list[idx] << "\n";
      }
    }

    void operator()(const ast::clear&) const {
      _history.clear();
    }
};


History::History()
: _idx{_history.end()}
{
}

History::~History() = default;


void History::add(const Line& command) {
  _history.emplace_back(command);
  _idx = _history.end();
}

const Line& History::forward(const Line& current) {
  return _idx < _history.end()
    ? _idx + 1 == _history.end() ? empty: *++_idx
    : _history.empty() ? current: _history.back();
}

const Line& History::backward(const Line& current) {
  return _idx > _history.begin()
    ? *--_idx
    : _history.empty() ? current: _history.front();
}

void History::clear() {
  _history.clear();
  _idx = _history.end();
}

const std::vector<Line>& History::list() const {
  return _history;
}

Command::Status History::execute(const Line& line, Output& out) {
  auto iter = line.begin();
  auto endIter = line.end();

  ast::HistoryCommand data;
  const bool ok {x3::phrase_parse(iter, endIter, historyCommand, x3::space, data)};

  if (!ok || iter != endIter) return Command::Status::NoMatch;

  if (data){
    boost::apply_visitor(Visitor{*this, out}, data.get());
  } else {
    out << "Execute last command" << "\n";
  }
  return Command::Status::Ok;
}

bool History::matches(const Line& line) const {
  auto iter = line.begin();
  auto endIter = line.end();

  const bool ok {x3::phrase_parse(iter, endIter, historyCommand, x3::space)};

  return ok || static_cast<size_t>(distance(line.begin(), iter)) == line.size();
}


Command::Suggestions History::suggestions(const Line& line) const {
  return {};
}
