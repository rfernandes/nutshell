// #define BOOST_SPIRIT_X3_DEBUG

#include <io/Cursor.h>
#include <io/Input.h>
#include <io/Output.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace boost::spirit::x3::traits {

template <>
void move_to<std::string::iterator, std::string_view>
            (std::string::iterator first,
             std::string::iterator last,
             std::string_view& view)
{
  view = std::string_view(&*first, static_cast<size_t>(std::distance(first, last)));
}

}

namespace helpers{
  std::string_view make_view(std::string::const_iterator begin,
                             std::string::const_iterator end){
    return {&*begin, static_cast<size_t>(std::distance(begin, end))};
  }
  std::string_view make_view(std::string_view::const_iterator begin,
                             std::string_view::const_iterator end){
    return {&*begin, static_cast<size_t>(std::distance(begin, end))};
  }
}



namespace {
  namespace x3 = boost::spirit::x3;
  template<typename T>
      auto as = [](auto p) { return x3::rule<struct _, T>{} = as_parser(p); };
}

struct Segment{
  enum class Type{
    Separator
  };
  Type type;
  std::string_view view;

  Segment(Type type,
          std::string::const_iterator begin,
          std::string::const_iterator end)
  : type{type}
  , view{helpers::make_view(begin, end)}
  {
  }

};

namespace ast {

  using Simple = std::string_view;

  struct File
  {
    std::string_view name;
  };

  struct Path
  {
    std::string_view name;
  };

  using Identifier = std::string_view;

  struct Variable
  {
    Identifier name;
  };

  struct String
  {
    std::vector<x3::variant<Variable, std::string_view>> value;
  };

  using Component = x3::variant<Variable, String, File, Path, Simple>;

  struct Executable
  {
    std::vector<Component> components;
  };

  using Pipe = std::vector<Executable>;
  using Command = std::vector<Pipe>;
}

BOOST_FUSION_ADAPT_STRUCT(ast::File, name)
BOOST_FUSION_ADAPT_STRUCT(ast::Path, name)
BOOST_FUSION_ADAPT_STRUCT(ast::String, value)
BOOST_FUSION_ADAPT_STRUCT(ast::Variable, name)
BOOST_FUSION_ADAPT_STRUCT(ast::Executable, components)

namespace parser{
  namespace x3 = boost::spirit::x3;

  // Command

  struct command_class {};
  using command_type = x3::rule<command_class, ast::Command>;
  BOOST_SPIRIT_DECLARE(command_type)
}


struct ParseResult
{
  using Segments = std::vector<Segment>;
  Segments segments;
  std::string line;
  ast::Command data;
  bool ok;
  bool complete;
};


namespace parser
{
  template<Segment::Type Type>
  struct type_annotation
  {
    template <typename Ast, typename Iterator, typename Context>
    inline void on_success(Iterator const& first,
                           Iterator const& last,
                           Ast& /*ast*/,
                           Context const& context){
      auto& desc = boost::spirit::x3::get<ParseResult>(context).get();
      desc.segments.emplace_back(Type, first, last);
    }
  };
}


namespace parser {
  struct SeparatorT: type_annotation<Segment::Type::Separator>{};
  using separator_type = x3::rule<SeparatorT>;
  using simple_type = x3::rule<struct SimpleT, ast::Simple>;
  using component_type = x3::rule<struct ComponentT, ast::Component>;
  using file_type = x3::rule<struct FileT, ast::File>;
  using path_type = x3::rule<struct PathT, ast::Path>;
  using identifier_type = x3::rule<struct IdentifierT, ast::Identifier>;
  using variable_type = x3::rule<struct VariableT, ast::Variable>;
  using string_type = x3::rule<struct stringT, ast::String>;
  using pipe_type = x3::rule<struct PipeT, ast::Pipe>;
  using executable_type = x3::rule<struct ExecutableT, ast::Executable>;

  const separator_type separator = "separator";
  const simple_type simple = "simple";
  const component_type component = "component";
  const file_type file = "file";
  const path_type path = "path";
  const identifier_type identifier = "identifier";
  const variable_type  variable = "variable";
  const string_type string = "string";
  const executable_type executable = "executable";
  const pipe_type pipe = "pipe";
  const command_type command = "command";

  auto special = x3::char_("\"$|&;{} ");

  auto separator_def = x3::lit(';');
  auto simple_def = x3::raw[+~special];
  auto file_def = x3::no_skip[':' >> simple];
  auto path_def = x3::no_skip[x3::raw[x3::char_("./") >> *(x3::char_("./") | ~special)]];
  auto identifier_def = x3::raw[x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))]];
  auto variable_def = x3::no_skip["${" >> identifier >> '}'];
  auto string_def = x3::no_skip['"' >> *(variable | as<std::string_view>(x3::raw[+~x3::char_("\"$")])) >> '"'];
  auto component_def = variable | string | file | path | simple;
  auto executable_def = as<std::vector<ast::Component>>(x3::no_skip[(+component) % +x3::lit(' ')]);
  auto pipe_def =  executable % ('|' >> *x3::space);
  auto command_def = pipe % (separator >> *x3::space);

  BOOST_SPIRIT_DEFINE(
    separator,
    simple,
    file,
    path,
    identifier,
    variable,
    string,
    component,
    executable,
    pipe,
    command
  )
}


namespace analyse{

  class PrintVisitor
  {
  public:
    PrintVisitor(std::ostream& out)
    : _out{out}
    {
    }

    void operator()(const std::string_view& str) const {
      _out << "sp:(" << str << ')';
    }

    void operator()(const ast::File& file) const {
      _out << "f:(" << file.name << ')';
    }

    void operator()(const ast::Path& path) const {
      _out << "p:(" << path.name << ')';
    }

    void operator()(const ast::String& string) const {
      _out << "s:(";
      for (const auto& part: string.value){
        boost::apply_visitor(*this, part.get());
      }
      _out << ')';
    }

    void operator()(const ast::Variable& variable) const {
      _out << "var:(" << variable.name << ')';
    }

  private:

    std::ostream& _out;
  };

  std::unordered_map<std::string, std::string> variables;

  class ExecutableVisitor
  {
  public:
    ExecutableVisitor(std::istream* in, std::ostream* out)
    : _in{in}
    , _out{out}
    {
    }

    std::unique_ptr<std::istream> operator()(const std::string_view& str) const {
      auto ret = std::make_unique<std::stringstream>();
      *ret << str;
      return ret;
    }

    std::unique_ptr<std::istream> operator()(const ast::File& file) const {
      if (_in){
        auto fout = std::make_unique<std::fstream>(std::string{file.name}, std::ios::in |std::ios::out | std::ios::trunc);
        *fout << _in->rdbuf();
      }
      auto fin = std::make_unique<std::ifstream>(std::string{file.name});
      return fin;
    }

    std::unique_ptr<std::istream> operator()(const ast::Path& path) const {
      std::filesystem::path dir{path.name};
      if (std::filesystem::is_directory(dir)){
        std::filesystem::current_path(dir);
      }
      auto ret = std::make_unique<std::istringstream>(std::filesystem::current_path());
      return ret;
    }

    std::unique_ptr<std::istream> operator()(const ast::String& string) const {
      if (_out){
        *_out << "s:(";
        for (const auto& part: string.value){
          boost::apply_visitor(*this, part.get());
        }
        *_out << ')';
      }
      return nullptr;
    }

    std::unique_ptr<std::istream> operator()(const ast::Variable& variable) const {
      if (_in){
        std::stringstream ss;
        ss << _in->rdbuf();
        variables[std::string{variable.name}] = ss.str();
      }
      auto it = variables.find(std::string(variable.name));
      auto ret = std::make_unique<std::istringstream>(it != variables.end() ? it->second: "");
      return ret;
    }

  private:

    std::istream* _in{nullptr};
    std::ostream* _out{nullptr};
  };

}

void execute(const ParseResult& result){
  // Create and use execution::ExecutableVisitor
  std::cerr << manip::Erase::CursorToBegin;
  Cursor cursor{std::cin, std::cerr};
  cursor.column(0);

  std::stringstream buf;

  std::unique_ptr<std::istream> in{nullptr};
  if (result.ok){
    for (const auto& pipe: result.data){
//       std::istream* in{&pipe != &*begin(result.data) ? &std::cin : nullptr};
      std::ostream* out{&pipe != &*end(result.data) ? &std::cout : nullptr};
      for (const auto& command: pipe){
        analyse::ExecutableVisitor visitor{in.get(), out};
        for (const auto& item: command.components){
          in = std::move(boost::apply_visitor(visitor, item.get()));
        }
      }
    }
  }
  if (in)
  {
    std::cout << in->rdbuf() << "\n";
  }
}

void parse(ParseResult& result, std::ostream& out){
  std::cerr << manip::Erase::CursorToBegin;
  Cursor cursor{std::cin, std::cerr};
  cursor.column(0);

  auto iter = result.line.begin();
  const auto &endIter = result.line.end();
  namespace x3 = boost::spirit::x3;

  result.data.clear();

  ParseResult desc;
  const auto command = x3::with<ParseResult>(std::ref(desc))[parser::command];

  const bool ok {x3::phrase_parse(iter, endIter, command, x3::space, result.data)};
  bool complete{iter == endIter};

  result.ok = ok;
  result.complete = complete;

  std::cerr << result.line << " -> " << " all:" << complete << " ok:" << ok  << " parse:";

  analyse::PrintVisitor visitor{std::cout};

  for (const auto& item: result.data) {
    std::cerr << "{";
    for (const auto& item2: item){
      std::cerr << "[";
      for (const auto& item3: item2.components){
        std::cerr << "<";
        boost::apply_visitor(visitor, item3.get());
        std::cerr << ">";
      }
      std::cerr << "]";
    }
    std::cerr << "}";
  }
}

int main (int argc, char*argv[])
{
  ParseResult parseResult;
  std::string& line = parseResult.line;

  Input in{std::cin};
  while (auto keyPress = in.get()) {
    switch (keyPress) {
      case '\n':
        std::cout << '\n';
        execute(parseResult);
        line.clear();
        continue;
        break;
      case Input::Backspace:
      case '\b': // Ctrl-H
        if (!line.empty()) {
          line.pop_back();
        }
        break;
      default:
        line.insert(line.size(), 1, keyPress);
        break;
    }
    parse(parseResult, std::cout);
  }

  return 0;
}
