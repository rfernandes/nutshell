#include "History.h"

#include <command/Command.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;

namespace ast {
  class list{};
  class clear{};
  class last{};

  using functions = boost::variant <clear, list, last>;
  using HistoryCommand = boost::variant <uint64_t, functions>;
}

namespace {
  namespace x3 = boost::spirit::x3;

  auto list = x3::rule<class list, ast::list>()
    = x3::lit("list") >> x3::attr(ast::list{});

  auto clear = x3::rule<class clear, ast::clear>()
    = x3::lit("clear") >> x3::attr(ast::clear{});

  auto last = x3::rule<class last, ast::last>()
    = x3::lit("last") >> x3::attr(ast::last{});

  auto functions = x3::rule<class functions, ast::functions>()
    =  clear | list | last;

  auto historyCommand = x3::rule<class historyCommand, ast::HistoryCommand>()
    = '!' >> ( x3::uint64 |
               '.' >> functions);

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

    void operator()(const ast::functions& function) const {
      return boost::apply_visitor(*this, function);
    }

    void operator()(const ast::list&) const {
      auto &list = _history.list();
      for (size_t idx=0; idx< list.size(); ++idx){
        _out << idx << ' ' << list[idx] << "\n";
      }
    }

    void operator()(const ast::clear&) const {
      _history.clear();
    }

    void operator()(const ast::last&) const {
      _out << "Unimplemented - Execute last command\n";
    }

    void operator()(uint64_t &idx) const {
      _out << "Unimplemented - Execute command idx" << idx << "\n";
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

  boost::apply_visitor(Visitor{*this, out}, data);

  return Command::Status::Ok;
}

bool History::matches(const Line& line) const {
  auto iter = line.begin();
  auto endIter = line.end();

  const bool ok {x3::phrase_parse(iter, endIter, historyCommand, x3::space)};

  return ok || static_cast<size_t>(distance(line.begin(), iter)) == line.size();
}


Command::Suggestions History::suggestions(const Line& /*line*/) const {
  return {};
}
