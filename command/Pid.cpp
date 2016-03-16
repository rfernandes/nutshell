#include "Pid.h"

#include <fstream>

#include <experimental/filesystem>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <signal.h>
#include <sys/wait.h>

using namespace std;
using namespace std::experimental::filesystem;

namespace {
  // TODO: move to Utils header
  string slurp(const istream& in){
    return static_cast<stringstream const&>(stringstream() << in.rdbuf()).str();
  }
}

namespace ast {
  using Pid = boost::variant<unsigned, string>;

  struct kill_function{
    boost::optional<uint8_t> signal;
  };

  using wait_function = uint64_t;

  struct cwd_function{};

  using functions = boost::variant<kill_function, wait_function, cwd_function>;

  struct pid_command{
    boost::optional<Pid> pid;
    boost::optional<functions> function;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::pid_command, pid, function)
BOOST_FUSION_ADAPT_STRUCT(ast::kill_function, signal)

namespace {
  namespace x3 = boost::spirit::x3;

  auto killFunction = x3::rule<class killFunction, ast::kill_function>()
    = x3::lit("kill") >>  ( '(' >> - x3::uint8 >> ')' );

  auto waitFunction = x3::rule<class waitFunction, ast::wait_function>()
    = x3::lit("wait") >> '(' >> x3::uint64 >> ')';

  auto cwdFunction =  x3::rule<class cwdFunction, ast::cwd_function>()
    = "cwd" >> x3::attr(ast::cwd_function{});

  auto functions = x3::rule<class functions, ast::functions>()
    = killFunction | waitFunction | cwdFunction;

  auto pidName = x3::rule<class pidName, string>()
    = '"' >> x3::no_skip[+~x3::char_('"')] >> '"' |
      +~x3::char_(' ');

  auto pidCommand = x3::rule<class pidCommand, ast::pid_command>()
    = '^' >> - (x3::uint_ | pidName) >> - x3::no_skip[ '.' >> functions ];

  struct PidVisitor {
    unsigned operator()(unsigned& i) const {
      return i;
    }

    unsigned operator()(std::string& str) const {
      for(auto& entry: directory_iterator{"/proc"}) {
        if (is_directory(entry) && !is_symlink(entry)) {
          ifstream fin{(entry.path() / "cmdline").string()};
          if (!fin){
            continue;
          }
          auto cmdline = slurp(fin);
          cmdline = cmdline.substr(0, cmdline.find_first_of('\0'));
          if (cmdline.find(str) != string::npos){
            return stoul(entry.path().stem());
          }
        }
      }
      // FIXME: when lists are implemented return all pids instead of just the first
      return 0;
    }
  };

  class FunctionVisitor {
    unsigned _pid;
    Output& _out;

  public:
    FunctionVisitor(unsigned pid, Output& output)
    : _pid{pid}
    , _out{output}
    {
    }

    void operator()(const ast::kill_function& function) const {
      auto signal = function.signal.get_value_or(6);
      kill(_pid, signal);
      _out << "killed [" << signal << "]: " << _pid << "\n";
    }

    void operator()(const ast::wait_function& /*function*/) const {
      _out << "waiting : " << _pid << "\n";
      wait(const_cast<unsigned*>(&_pid));
    }

    void operator()(const ast::cwd_function&) const {
      path pidCwd{"/proc"};
      pidCwd /= to_string(_pid) / "cwd";
      // TODO: Read_symlink isn't working with /proc symlink ??
      _out << read_symlink(pidCwd).string() << "\n";
    }
  };
}

bool Pid::matches(const Line & line) const {
  const string command {line()};
  auto iter = command.begin();
  auto endIter = command.end();

  const bool ok {x3::phrase_parse(iter, endIter, pidCommand, x3::space)};

  return ok || static_cast<size_t>(distance(command.begin(), iter)) == command.size();
}

Command::Suggestions Pid::suggestions(const Line & /*line*/) const {
  return {};
}

Command::Status Pid::execute(const Line & line, Output & output){
  const string command {line()};
  auto iter = command.begin();
  auto endIter = command.end();

  ast::pid_command data;
  const bool ok {x3::phrase_parse(iter, endIter, pidCommand, x3::space, data)};

  if (!ok) return Command::Status::NoMatch;

  const unsigned pid = data.pid ? boost::apply_visitor(PidVisitor{}, data.pid.get())
                                : getpid();

  if (data.function) {
    boost::apply_visitor(FunctionVisitor{pid, output}, data.function.get());
  } else {
    output << pid << "\n";
  }
  return Command::Status::Ok;
}

namespace {
  auto pid = CommandStore::store<Pid>();
}
