#include "Executable.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <unistd.h>
#include <sys/wait.h>

using namespace std;
using namespace std::experimental;

namespace x3 = boost::spirit::x3;

namespace {
  enum PipeDescriptors {
    READ  = 0,
    WRITE = 1
  };

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
  struct option_class: parser::type_annotation<Segment::Type::Parameter>{};
  struct longOption_class: parser::type_annotation<Segment::Type::Argument>{};
  struct parameter_class: parser::type_annotation<Segment::Type::Builtin>{};
  struct executable_class: parser::type_annotation<Segment::Type::Command>{};
  struct command_class;

  using option_type = x3::rule<option_class, ast::Parameters>;
  using longOption_type = x3::rule<longOption_class, ast::Parameters>;
  using parameter_type = x3::rule<parameter_class, ast::Parameters>;
  using executable_type = x3::rule<executable_class, ast::Executable>;
  using command_type = x3::rule<command_class, ast::Command>;

  const option_type option = "option";
  const longOption_type longOption = "longOption";
  const parameter_type parameter = "parameter";
  const executable_type executable = "executable";
  const command_type command = "command";

  auto option_def = x3::lexeme[x3::char_('-') >> +~x3::char_(' ')];
  auto longOption_def = x3::lexeme[x3::string("--") >> +~x3::char_(' ')];
  auto parameter_def = x3::lexeme[+~x3::char_(' ')];
  auto executable_def = x3::lexeme[+~x3::char_(' ')];
  auto command_def = executable >> *(longOption | option | parameter);

  BOOST_SPIRIT_DEFINE(
    option,
    longOption,
    parameter,
    executable,
    command
  )

  // Pipe to capture output
  void launch(const ast::Command &command, Output& output) {
    int toChild[2];
    int fromChild[2];

    // FIXME: check for non null return
    pipe(toChild);
    pipe(fromChild);

    switch (auto pid=fork()){
      case -1:
        throw std::runtime_error("Unable to fork");
        break;
      case 0: { // Child
        dup2(toChild[READ], STDIN_FILENO);
        dup2(fromChild[WRITE], STDOUT_FILENO);
        dup2(fromChild[WRITE], STDERR_FILENO);
        close(toChild[WRITE]);
        close(fromChild[READ]);
        std::vector<const char*> args;
        args.emplace_back(command.command.c_str());
        std::transform(command.parameters.begin(),
                       command.parameters.end(),
                       std::back_inserter(args), [](auto &str){
                         return str.data();
                       });
        args.emplace_back(nullptr);
        execvp(command.command.data(), const_cast<char*const*>(&args[0]));
        std::terminate();
        break;
      }
      default: // Parent
        const auto bufferSize = 100;
        char buffer[bufferSize+1];
        close(toChild[READ]);
        close(fromChild[WRITE]);
        int status;
        bool finished = false;
        while (!finished) {
          switch (auto readResult = read(fromChild[READ], buffer, bufferSize)){
            case 0: /* End-of-File, or non-blocking read. */
              waitpid(pid, &status, 0);
              finished = true;
              break;
            case -1:
              if (errno == EINTR ||
                  errno == EAGAIN){
                errno = 0;
                break;
              }else{
                throw std::runtime_error("Reading from child faild");
              }
            default:
              output.write(buffer, readResult);
              break;
          }
        }
        break;
    };
  }
}

Executable::Executable(std::set<std::string> paths)
: _paths{std::move(paths)}
{
}

Description Executable::parse(const Line& line, Output& output, bool execute){
  auto iter = line.begin();
  const auto& endIter = line.end();

  Description desc{line};
  const auto parser = x3::with<Description>(ref(desc))[
                        x3::with<Line>(ref(line))[command]];

  ast::Command data;
  bool ok {
    x3::phrase_parse(iter, endIter, parser, x3::space, data) &&
    // Move this into semantic action
    _paths.count(data.command)
  };

  if (ok){
    desc.status(Status::Ok);

    if (execute){
      launch(data, output);
    }
  }

  return desc;
}
