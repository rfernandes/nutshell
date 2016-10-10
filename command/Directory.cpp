#include "Directory.h"

#include <command/BuiltIn.h>
#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <pwd.h>

using namespace std;
using namespace std::experimental::filesystem;

namespace x3 = boost::spirit::x3;

namespace ast {
  using path = std::string;

  struct previous{};
  struct next{};

  struct functions: x3::variant<previous, next, string>{
    using base_type::base_type;
    using base_type::operator=;
  };

  struct CdCommand{
    boost::optional<functions> parameters;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::CdCommand, parameters)

namespace {
  struct previous_class: parser::type_annotation<Segment::Type::Function>{};
  struct next_class: parser::type_annotation<Segment::Type::Function>{};
  struct path_class: parser::type_annotation<Segment::Type::Parameter> {};
  struct functions_class{};
  struct commandName_class: parser::type_annotation<Segment::Type::Builtin>{};
  struct command_class{};

  using previous_type = x3::rule<previous_class, ast::previous>;
  using next_type = x3::rule<next_class, ast::next>;
  using path_type = x3::rule<path_class, ast::path>;
  using functions_type = x3::rule<functions_class, ast::functions>;
  using commandName_type = x3::rule<commandName_class>;
  using command_type = x3::rule<command_class, ast::CdCommand>;

  const previous_type previous = "previous";
  const next_type next = "next";
  const path_type pathParam = "path";
  const functions_type functions = "functions";
  const commandName_type commandName = "commandName";
  const command_type command = "command";

  auto previous_def = '-' >> x3::attr(ast::previous{});
  auto next_def = "+" >> x3::attr(ast::next{});
  auto pathParam_def = +x3::char_;
  auto functions_def = previous | next | pathParam;
  auto commandName_def = "cd";
  auto command_def = commandName >> -functions;

  BOOST_SPIRIT_DEFINE(
    previous,
    next,
    pathParam,
    functions,
    commandName,
    command
  )
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
, _home{getpwuid(geteuid())->pw_dir}
, _history(1, _current)
, _idx{0}
{
  CommandStore::store<BuiltIn>(":cwd",
                               [this](const Line& /*line*/, Output& output){
                                 output << cwd().string();
                                 return Status::Ok;
                               });
}

ParseResult Cd::parse(const Line& line, Output& /*output*/, bool execute){
  auto iter = line.begin();
  const auto& endIter = line.end();


  ParseResult desc;
  const auto parser = x3::with<ParseResult>(ref(desc))[command];

  ast::CdCommand data;
  bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};

  if (!ok) return desc;

  desc.status(Status::Ok);

  if (execute){
    path target {data.parameters ? boost::apply_visitor(CdVisitor{*this}, data.parameters.get())
                                : _home};

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
  }

  return desc;
}

const path& Cd::cwd() const {
  return _current;
}

const path& Cd::home() const {
  return _home;
}

