#include "Pid.h"

#include "Command.h"
#include "Output.h"

#include <algorithm>
#include <unistd.h>

#include <sys/types.h>
#include <signal.h>

#include <experimental/filesystem>

#include <boost/spirit/home/x3.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/io.hpp>

using namespace std;
using namespace std::experimental::filesystem;

namespace ast {

  using Pid = boost::variant<unsigned, string>;

  struct pid_command{
    boost::optional<Pid> pid;
    boost::optional<string> function;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::pid_command, pid, function)

namespace {
  namespace x3 = boost::spirit::x3;

  auto functions = x3::rule<class functions, std::string>()
    = x3::string("kill") |
      x3::string("wait") |
      x3::string("cwd");

  template<typename T>
    auto as = [](auto p) { return x3::rule<struct _, T>{} = x3::as_parser(p); };

  auto pidCommand = x3::rule<class pidCommand, ast::pid_command>()
    = '^' >> -x3::uint_ >> -functions;


  struct pidVisitor {
    unsigned operator()(unsigned& i) const {
      return i;
    }

    unsigned operator()(std::string& /*str*/) const {
      // TODO: get pid by name, return list of pids instead
      return 31337;
    }
  };

}

bool Pid::matches(const Line & line) const {
  string command = line();
  auto iter = command.begin();
  auto endIter = command.end();

  bool ok {x3::phrase_parse(iter, endIter, pidCommand, x3::space)};

  return ok || static_cast<size_t>(distance(command.begin(), iter)) == command.size();
}

Command::Suggestions Pid::suggestions(const Line & /*line*/) const {
  return {};
}

Command::Status Pid::execute(const Line & line, Output & curses){
  string command = line();

  auto iter = command.begin();
  auto endIter = command.end();

  ast::pid_command data;
  bool ok {x3::phrase_parse(iter, endIter, pidCommand, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  unsigned pid = data.pid ? boost::apply_visitor(pidVisitor{}, data.pid.get())
                          : getpid();

  if (data.function) {
    const string& function = data.function.get();

    // TODO: Map this to enum
    if (function == "kill") {
      kill(pid, 15);
      curses << "killed [15]: " << pid << "\n";
    } else if (function == "cwd" ){
      path pidCwd{"/proc"};
      pidCwd /= to_string(pid) / "cwd";
      curses << read_symlink(pidCwd).string() << "\n";
    } else {
      curses << data.function.get() << " not implemented" << "\n";
    }
  } else { // Just print pid << "\n";
    curses << pid;
  }
  return Command::Status::Ok;
}

namespace {
  auto pid = CommandStore::store<Pid>();
}
