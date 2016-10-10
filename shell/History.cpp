#include "History.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <chrono>
#include <iomanip>

#include <pwd.h>

using namespace std;
using namespace std::experimental;
namespace x3 = boost::spirit::x3;

namespace {
  auto &history = ModuleStore::store<History>();

  class HistoryCommand: public Command{
    History &_history;
  public:
    HistoryCommand(History &history)
    : _history{history}
    {}

    ParseResult parse(const Line & line, Output & output, bool execute) override;
  };

  auto &historyCommand = CommandStore::store<HistoryCommand>(history);

  namespace ast {
    struct list{};
    struct clear{};
    struct prev{};

    struct functions: x3::variant<clear, list, prev>{
      using base_type = base_type;
      using base_type::operator=;
    };
    using Command = boost::variant <uint64_t, functions>;
  }

  struct list_class {};
  struct clear_class {};
  struct prev_class {};
  struct functions_class: parser::type_annotation<Segment::Type::Function>{};
  struct sigil_class: parser::type_annotation<Segment::Type::Builtin>{};
  struct command_class{};

  using list_type = x3::rule<list_class, ast::list>;
  using clear_type = x3::rule<clear_class, ast::clear>;
  using prev_type = x3::rule<prev_class, ast::prev>;
  using functions_type = x3::rule<functions_class, ast::functions>;
  using sigil_type = x3::rule<sigil_class>;
  using command_type = x3::rule<command_class, ast::Command>;

  const list_type list = "list";
  const clear_type clear = "clear";
  const prev_type prev = "prev";
  const functions_type functions = "functions";
  const sigil_type sigil = "sigil";
  const command_type command = "command";

  auto list_def = "list" >> x3::attr(ast::list{});
  auto clear_def = "clear" >> x3::attr(ast::clear{});
  auto prev_def = "prev" >> x3::attr(ast::prev{});
  auto functions_def = "." >> (clear | list | prev);
  auto sigil_def = "!";
  auto command_def = sigil >> (functions | x3::uint64);

  BOOST_SPIRIT_DEFINE(
    list,
    clear,
    prev,
    functions,
    sigil,
    command
  )

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
        const auto entry = list.at(idx);
        const auto start = std::chrono::system_clock::to_time_t(entry.startTime);
        const auto end = std::chrono::system_clock::to_time_t(entry.endTime);
        _out << idx << ' ' << entry.line << " " << put_time(localtime(&start), "%F %T") << " " << put_time(localtime(&end), "%F %T") << "\n";
      }
    }

    void operator()(const ast::clear&) const {
      _history.clear();
    }

    void operator()(const ast::prev&) const {
      _out << "Unimplemented - Execute prev command\n";
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


void History::add(const Entry& entry) {
  _history.push_back(entry);
  _idx = _history.end();
}

const Line& History::forward(const Line& current) {
  return _idx < _history.end()
    ? _idx + 1 == _history.end() ? empty: (++_idx)->line
    : _history.empty() ? current: _history.back().line;
}

const Line& History::backward(const Line& current) {
  return _idx > _history.begin()
    ? (--_idx)->line
    : _history.empty() ? current: _history.front().line;
}

void History::clear() {
  _history.clear();
  _idx = _history.end();
}

const std::vector<History::Entry>& History::list() const {
  return _history;
}

string_view History::suggest(const Line& line) const {
  string_view ret;
  if (!line.empty()){
    for (const auto& item: _history){
      if (item.line.compare(0, line.length(), line) == 0) {
        ret = item.line;
      }
    }
  }
  return ret;
}

ParseResult HistoryCommand::parse(const Line& line, Output& output, bool execute){
  auto iter = line.begin();
  auto endIter = line.end();

  ParseResult desc;
  const auto parser = x3::with<ParseResult>(ref(desc))[command];

  ast::Command data;
  const bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};

  if (ok){
    desc.status(Status::Ok);
    if (execute){
      boost::apply_visitor(Visitor{_history, output}, data);
    }
  }

  return desc;
}

void History::commandExecute(const Line& /*line*/, Shell& /*shell*/){
  _startTime = std::chrono::system_clock::now();
}

void History::commandExecuted(const ParseResult& /*parseResult*/, Shell& shell){
  const auto endTime = std::chrono::system_clock::now();
  // FIXME: Capture correct return status
  const Entry entry{shell.line(), 0, _startTime, endTime};
  add(entry);
}

bool History::keyPress(unsigned int keystroke, Shell& shell){
  switch (keystroke){
    case Input::Up:
    case Input::Down: {
      const auto& line = shell.line();
      const auto& newLine = keystroke == Input::Down ? forward(line) : backward(line);
      shell.line(newLine);
      shell.displayLine();
      return true;
    }
    case 18: //Ctrl-R
      cout << "History interactive mode unimplemented\n";
      return true;
  }
  return false;
}
