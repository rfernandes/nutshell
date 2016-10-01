#include "Pid.h"
#include <command/Parser.h>

#include <fstream>
#include <experimental/filesystem>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <signal.h>
#include <sys/wait.h>

using namespace std;
using namespace std::experimental::filesystem;

namespace x3 = boost::spirit::x3;

namespace {
  // TODO: move to Utils header
  string slurp(const istream& in){
    return static_cast<stringstream const&>(stringstream() << in.rdbuf()).str();
  }
}

namespace ast {
  using Pid = boost::variant<unsigned, string>;

  struct kill{
    boost::optional<uint8_t> signal;
  };

  struct wait{
    uint64_t seconds;
  };

  struct cwd{};

  using name = string;

  using functions = boost::variant<kill, wait, cwd>;

  struct PidCommand{
    boost::optional<Pid> pid;
    boost::optional<functions> function;
  };
}

BOOST_FUSION_ADAPT_STRUCT(ast::PidCommand, pid, function)
BOOST_FUSION_ADAPT_STRUCT(ast::kill, signal)
BOOST_FUSION_ADAPT_STRUCT(ast::wait, seconds)

namespace {

  struct kill_class {};
  struct wait_class {};
  struct cwd_class {};
  struct functions_class: parser::type_annotation<Segment::Type::Function>{};
  struct sigil_class: parser::type_annotation<Segment::Type::Builtin>{};
  struct command_class{};

  using kill_type = x3::rule<kill_class, ast::kill>;
  using wait_type = x3::rule<wait_class, ast::wait>;
  using cwd_type = x3::rule<cwd_class, ast::cwd>;
  using functions_type = x3::rule<functions_class, ast::functions>;
  using name_type = x3::rule<cwd_class, ast::name>;
  using sigil_type = x3::rule<sigil_class>;
  using command_type = x3::rule<command_class, ast::PidCommand>;

  const kill_type kill = "kill";
  const wait_type wait = "wait";
  const cwd_type cwd = "cwd";
  const functions_type functions = "functions";
  const name_type name = "name";
  const sigil_type sigil = "sigil";
  const command_type command = "command";

  auto kill_def = x3::lit("kill") >> -x3::uint8;
  auto wait_def = x3::lit("wait") >> x3::uint64;
  auto cwd_def = "cwd" >> x3::attr(ast::cwd{});
  auto functions_def = kill | wait | cwd;
  auto name_def = '"' >> x3::no_skip[+~x3::char_('"')] >> '"' | x3::alpha >> *x3::alnum;
  auto sigil_def = '^';
  auto command_def = sigil >> -(x3::uint_ | name) >> - x3::no_skip[ '.' >> functions ];

  BOOST_SPIRIT_DEFINE(
    kill,
    wait,
    cwd,
    functions,
    name,
    sigil,
    command
  )

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

    void operator()(const ast::kill& function) const {
      auto signal = function.signal.get_value_or(6);
      ::kill(_pid, signal);
      _out << "killed [" << signal << "]: " << _pid << "\n";
    }

    void operator()(const ast::wait& /*function*/) const {
      _out << "waiting : " << _pid << "\n";
      int pid = _pid; //FIXME: Narrowing
      ::wait(&pid);
    }

    void operator()(const ast::cwd&) const {
      path pidCwd{"/proc"};
      pidCwd /= to_string(_pid) / "cwd";
      // TODO: Read_symlink isn't working with /proc symlink ??
      _out << read_symlink(pidCwd).string() << "\n";
    }
  };
}

Description Pid::parse(const Line& line, Output& output, bool execute) {
  auto iter = line.begin();
  auto endIter = line.end();

  Description desc{line};
  const auto parser = x3::with<Description>(ref(desc))[command];


  ast::PidCommand data;
  const bool ok {x3::phrase_parse(iter, endIter, parser, x3::space, data)};

  if (!ok) return desc;

  const unsigned pid = data.pid ? boost::apply_visitor(PidVisitor{}, data.pid.get())
                                : getpid();

  desc.status(Status::Ok);

  if (execute) {
    if (data.function) {
      boost::apply_visitor(FunctionVisitor{pid, output}, data.function.get());
    } else {
      output << pid << "\n";
    }
  }
  return desc;
}

namespace {
  auto& pid = CommandStore::store<Pid>();
}
