#include "Executable.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

using namespace std;
using namespace std::experimental;

namespace x3 = boost::spirit::x3;

namespace {
  // Pipe to capture output
  Status launch(const Line &line, Output& out) {
    unique_ptr<FILE, decltype(&pclose)> pipe{popen(line.data(), "r"), pclose};
    if (!pipe){
      throw std::runtime_error("Unable to fork/pipe");
    }
    constexpr size_t bufferSize{1024};
    char buffer[bufferSize];
    while (!feof(pipe.get())) {
      if (fgets(buffer, bufferSize, pipe.get()) != NULL) {
        out << buffer;
      }
    }
    return Status::Ok;
  }
  namespace ast {
    using Parameters = string;
    using Executable = string;

    struct Command{
      Executable command;
      vector<Parameters>  parameters;
    };
  }
}

BOOST_FUSION_ADAPT_STRUCT(ast::Command, command, parameters)


namespace {
  struct parameter_class: parser::type_annotation<Segment::Type::Parameter>{};
  struct executable_class: parser::type_annotation<Segment::Type::Command>{};
  struct command_class;

  using parameter_type = x3::rule<parameter_class, ast::Parameters>;
  using executable_type = x3::rule<executable_class, ast::Executable>;
  using command_type = x3::rule<command_class, ast::Command>;

  const parameter_type parameter = "parameter";
  const executable_type executable = "executable";
  const command_type command = "command";

  auto parameter_def = x3::lexeme[+~x3::char_(' ')];
  auto executable_def = x3::lexeme[+~x3::char_(' ')];
  auto command_def = executable >> *parameter;

  BOOST_SPIRIT_DEFINE(
    parameter,
    executable,
    command
  )
}

Executable::Executable(std::set<std::string> paths)
: _paths{std::move(paths)}
{
}

Description Executable::parse(const Line& line, Output& output, bool execute){
  auto iter = line.begin();
  const auto& endIter = line.end();

  Description desc;
  const auto parser = x3::with<Description>(ref(desc))[
                        x3::with<Line>(ref(line))[command]];

  ast::Command data;
  bool ok {
    x3::phrase_parse(iter, endIter, parser, x3::space, data) &&
    _paths.count(data.command)
  };

  if (ok){
    desc.status = Status::Ok;

    if (execute){
      launch(line, output);
    }
  }

  return desc;
}
