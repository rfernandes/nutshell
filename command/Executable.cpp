#include "Executable.h"

#include <command/Parser.h>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <csignal>
#include <experimental/filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>

using namespace std;
using namespace std::experimental::filesystem;

namespace x3 = boost::spirit::x3;

namespace {
  enum PipeDescriptors {
    READ  = 0,
    WRITE = 1
  };

  namespace ast {
    using Parameters = string;
    using Executable = string;
    using FileName = string;

    struct Command{
      Executable command;
      vector<Parameters>  parameters;
    };
  }

  bool is_executable (const path& file) {
    return is_regular_file(file) &&
      static_cast<bool>(
        directory_entry{file}.status().permissions() &
          (perms::owner_exec |
           perms::group_exec |
           perms::others_exec));
  }

  [[noreturn]]
  void launch(const ast::Command& command)
  {
      std::vector<const char*> args;
      args.emplace_back(command.command.c_str());
      std::transform(command.parameters.begin(),
                      command.parameters.end(),
                      std::back_inserter(args), [](auto &str){
                        return str.data();
                      });
      args.emplace_back(nullptr);
      execvp(args[0], const_cast<char*const*>(&args[0]));
      std::terminate();
  }
}

BOOST_FUSION_ADAPT_STRUCT(ast::Command, command, parameters)

namespace {
  struct option_class: parser::type_annotation<Segment::Type::Parameter>{};
  struct longOption_class: parser::type_annotation<Segment::Type::Parameter>{};
  struct argument_class: parser::type_annotation<Segment::Type::Argument>{};
  struct fileName_class: parser::type_annotation<Segment::Type::FileName>{};
  struct executable_class: parser::type_annotation<Segment::Type::Command>{};
  struct command_class;

  using option_type = x3::rule<option_class, ast::Parameters>;
  using longOption_type = x3::rule<longOption_class, ast::Parameters>;
  using argument_type = x3::rule<argument_class, ast::Parameters>;
  using fileName_type = x3::rule<fileName_class, ast::FileName>;
  using executable_type = x3::rule<executable_class, ast::Executable>;
  using command_type = x3::rule<command_class, ast::Command>;

  const option_type option = "option";
  const longOption_type longOption = "longOption";
  const argument_type argument = "argument";
  const fileName_type fileName = "fileName";
  const executable_type executable = "executable";
  const command_type command = "command";

  auto option_def = x3::lexeme[x3::char_('-') >> +~x3::char_(' ')];
  auto longOption_def = x3::lexeme[x3::string("--") >> +~x3::char_(' ')];
  auto argument_def = x3::lexeme[+~x3::char_(' ')];
  auto fileName_def = x3::lexeme[(x3::char_('.') | x3::char_('/')) >> *~x3::char_(' ')];
  auto executable_def = x3::lexeme[+~x3::char_(' ')];
  auto command_def = executable >> *(longOption | option | fileName | argument);

  BOOST_SPIRIT_DEFINE(
    option,
    longOption,
    argument,
    fileName,
    executable,
    command
  )

}

Executable::Executable(std::set<std::string> paths)
: _paths{std::move(paths)}
{
}

ParseResult Executable::parse(const Line& line, Output& output){
  auto iter = line.begin();
  const auto& endIter = line.end();

  ParseResult desc;
  const auto parser = x3::with<ParseResult>(ref(desc))[command];

  ast::Command data;
  bool ok {
    x3::phrase_parse(iter, endIter, parser, x3::space, data) &&
    // Move this into semantic action
    (_paths.count(data.command) || is_executable(directory_entry{data.command}))
  };

  if (ok){
    desc.data(data);
    desc.status(Status::Ok);
  }

  return desc;
}

// Pipe to capture output
void Executable::execute(const ParseResult& parseResult, Output& output) {

  int fdm, fds, rc;

  fdm = posix_openpt(O_RDWR);
  if (fdm < 0) {
    throw std::runtime_error("Error on posix_openpt");
    return;
  }

  rc = grantpt(fdm);
  if (rc != 0) {
    throw std::runtime_error("Error on grantpt");
    return;
  }

  rc = unlockpt(fdm);
  if (rc != 0) {
    throw std::runtime_error("Error on unlockpt");
    return;
  }

  // Open the slave side ot the PTY
  fds = open(ptsname(fdm), O_RDWR);

  switch (/*auto pid=*/fork()){
    case -1:
      throw std::runtime_error("Unable to fork");
      break;
    case 0: { // Child
      // Close the master side of the PTY
      close(fdm);

      // Save the default parameters of the slave side of the PTY

      // Enable RAW mode
      struct termios curr_term;
      rc = tcgetattr(fds, &curr_term);
      struct termios new_term{curr_term};
      cfmakeraw (&new_term);
      tcsetattr (fds, TCSANOW, &new_term);

      // The slave side of the PTY becomes the standard input and outputs of the child process
      close(0); // stdin
      close(1); // stdout
      close(2); // stderr

      dup(fds); // use pty stdin (0)
      dup(fds); // use pty stdout (1)
      dup(fds); // use pty stderr (2)

      // Now the original file descriptor is useless
      close(fds);

      // Make the current process a new session leader
      setsid();

      // As the child is a session leader, set the controlling terminal to be the slave side of the PTY
      // (Mandatory for programs like the shell to make them manage correctly their outputs)
      ioctl(0, TIOCSCTTY, 1);

      launch(std::any_cast<ast::Command>(parseResult.data()));
      break;
    }
    default: { // Parent
      const auto bufferSize{256};
      char buffer[bufferSize+1];

      fd_set fd_in;
      // Close the slave side of the PTY
      close(fds);

      bool finished{false};
      while (!finished) {
        // Wait for data from standard input and master side of PTY
        FD_ZERO(&fd_in);
        FD_SET(0, &fd_in);
        FD_SET(fdm, &fd_in);

        switch (rc = select(fdm + 1, &fd_in, nullptr, nullptr, nullptr)){
          case -1:
            if (errno == EINTR || errno == EAGAIN) {
              break;
            }
            throw std::runtime_error("Reading from child faild");
          default: {
            // If data on standard input
            if (FD_ISSET(0, &fd_in)) {
              rc = read(0, buffer, bufferSize);
              if (rc > 0) {
                // Send data on the master side of PTY
                write(fdm, buffer, rc);
              } else {
                throw std::runtime_error{"Error on read standard input"};
              }
            }

            // If data on master side of PTY
            if (FD_ISSET(fdm, &fd_in)) {
              rc = read(fdm, buffer, bufferSize);
              if (rc > 0) {
                // Send data on standard output
                write(1, buffer, rc);
              } else {
                if (errno == EIO) {
                  return;
                }
                throw std::runtime_error{"Error on read master PTY"};
              }
            }
            break;
          }
        }
      }
      break;
    }
  };
}
