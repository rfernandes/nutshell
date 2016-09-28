#include "Function.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;

namespace x3 = boost::spirit::x3;

namespace {
  namespace ast {
    using Name = string;

    struct assign{
      string content;
      bool finished;
    };

    struct Command{
      Name name;
      boost::optional<assign> content;
    };
  }
}

BOOST_FUSION_ADAPT_STRUCT(ast::assign, content, finished)
BOOST_FUSION_ADAPT_STRUCT(ast::Command, name, content)

namespace {
  struct assign_class{};
  struct name_class: parser::type_annotation<Segment::Type::Function>{};
  struct command_class{};

  using assign_type = x3::rule<assign_class, ast::assign>;
  using name_type = x3::rule<name_class, ast::Name>;
  using command_type = x3::rule<command_class, ast::Command>;

  const assign_type assign = "assign";
  const name_type name = "name";
  const command_type command = "command";

  auto assign_def = '{' >> x3::lexeme[ *(~x3::char_('}')) ] >> -('}' >> x3::attr(bool{true}));
  auto name_def = x3::lexeme[ -x3::char_(":") >>  x3::alpha >> *(x3::alnum | x3::char_("_-")) ];
  auto command_def = name >> -assign;

  BOOST_SPIRIT_DEFINE(
    assign,
    name,
    command
  )

  const auto match_name = [](auto &ctx, Function::Functions& functions) {
    const ast::Command& data = x3::_attr(ctx);
    if (!data.content && !functions.count(data.name)){
      x3::_pass(ctx) = false;
    }
  };
}

Function::Function(Functions functions)
: _functions{move(functions)}
{}

Description Function::parse(const Line& line, Output& output, bool execute){
  auto iter = line.begin();
  const auto& endIter = line.end();

  Description desc;
  const auto parser = x3::with<Description>(ref(desc))[
                        x3::with<Line>(ref(line))[command]];

  using namespace std::placeholders;
  auto matcher = [&](auto &ctx){
    return match_name(ctx, _functions);
  };

  ast::Command data;
  bool ok {x3::phrase_parse(iter, endIter, parser[matcher], x3::space, data)};

  if (ok) {
    desc.status = Status::Ok;
    if (execute){
      if (data.content){
        if (data.content.get().finished){
          _functions[data.name] = data.content.get().content;
        }else{
          desc.status = Status::Incomplete;
        }
      }else{
        // TODO: assign through semantic action so we don't have to loopup twice
        const auto& function = _functions.at(data.name);
        stringstream functionBody{function};
        string line;
        while (getline(functionBody, line)) {
          CommandStore::instance().parse(line, output, true);
        }
      }
    }
  }
  return desc;
}

