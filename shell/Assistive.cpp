#include "Assistive.h"

#include <core/BuiltIn.h>
#include <core/Utils.h>
#include <io/Output.h>
#include <shell/Shell.h>
#include <shell/History.h>

#include <experimental/filesystem>
#include <unordered_map>
#include <string_view>

using namespace std;
using namespace std::experimental::filesystem;
using namespace manip;

namespace {

  auto &assistive = ModuleStore::store<Assistive>();

  class HelpDatabase{
  public:

    struct CommandDescription
    {
      string description;
      unordered_map<string, string> parameters;
    };

    struct ParameterDescription
    {
      string parameter;
      string description;
    };

    unordered_map <string, CommandDescription> _db;

    HelpDatabase()
    :_db{
      {"ls", {"List information about the FILEs (the current directory by default).",{
          {"--color", "colorize the output"},
          {"-a", "do not ignore entries starting with ."},
          {"-all", "do not ignore entries starting with ."},
          {"-A", "do not list implied . and .."},
          {"--almost-all", "do not list implied . and .."},
          {"-l", "use a long listing format"},
          {"-r", "reverse order while sorting"},
      }}},
      {"git", {"Distributed source control", {
          {"add", "Add file contents to the index"},
          {"mv", "Move or rename a file, a directory, or a symlink"},
          {"reset", "Reset current HEAD to the specified state"},
          {"checkout", "Switch branches or restore working tree files"},
      }}}
    }
    {}

    string segmentDescription(Segment segment){
      string description;
      switch (segment.type){
        case Segment::Type::Builtin:
          description = segment.info;
          break;
        case Segment::Type::FileName:
          if (exists(segment.view)) {
            description = "known file";
            break;
          }
          [[fallthrough]];
        default:
          description = string{"unknown "} + enum_cast<const char *>(segment.type);
      }
      return description;
    }

    vector<string> describe(const ParseResult& parseResult){
      vector<string> ret;

      const auto &segments = parseResult.segments();
      if (!segments.empty()){
        const auto &command = segments.front().view;

        const auto commandIt = _db.find(string{command});
        if (commandIt != _db.end()){
          // Store command description
          ret.emplace_back(commandIt->second.description);
          // Store segment description
          for (const auto& segment: segments){
            if (&segment != &segments.front()){
              auto parameterIt = commandIt->second.parameters.find(string{segment.view});
              if (parameterIt != commandIt->second.parameters.end()){
                ret.emplace_back(parameterIt->second);
              }else{
                ret.push_back(std::move(segmentDescription(segment)));
              }
            }
          }
        }else{
          for (const auto& segment: segments){
            ret.push_back(std::move(segmentDescription(segment)));
          }
        }
      }
      return ret;
    }
  };


  ostream& operator << (ostream& out, Segment::Type type){
    switch (type){
      case Segment::Type::Builtin:
        out << Color::Cyan;
        break;
      case Segment::Type::Command:
        out << Color::Green;
        break;
      case Segment::Type::Parameter:
        out << Color::Blue;
        break;
      case Segment::Type::Argument:
        out << Color::Magenta;
        break;
      case Segment::Type::FileName:
        out << Mode::Bold << Color::Magenta;
        break;
      case Segment::Type::String:
        out << Mode::Bold << Color::Blue;
        break;
      case Segment::Type::Function:
        out << Color::Yellow;
        break;
      default:
        out << Color::White;
    }
    return out;
  }

  ostream& operator << (ostream& out, const Segment& segment){
    out << segment.type << segment.view;
    return out;
  }

  HelpDatabase help;

  /* TODO:
  *    Clear &/Pad when executed
  *    Compact view (maximise line information)
  */
  ostream & assist(ostream &out,
                   const ParseResult::Segments &segments,
                   const vector<string>& description){
    for (size_t count = segments.size(); count > 0; --count){
      auto it = segments.front().view.begin();
      for (size_t i = 0; i < count; ++i) {
        const auto &segment = segments.at(i);
        out << repeat(' ', distance(it, segment.view.begin()));
        it = segment.view.begin();
        if (i + 1 == count){
          out << "╰" << segment.type << "╸ " << description[i];
        }else{
          out << "│";
          ++it;
        }
      }
      out << Color::Reset << '\n';
    }
    return out;
  }
}

Assistive::Assistive()
:_active{false}
{
  // Toggle predictive functionality
  CommandStore::store<BuiltIn>(":assistive",
                               "Toggle as-you-type token parseResults",
                              [=](const Line& /*line*/, Output& /*output*/){
                                toggle();
                                return Status::Ok;
                              });
}

void Assistive::lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell& shell){
  auto& out = shell.out();

  if (!_active){
    out << line.line() << Color::Reset;

  }else{
    switch (parseResult.status()){
      case Status::NoMatch:
        out << Color::Red << line.line() << Color::Reset;
        break;
      default:{
        auto lineView = string_view{line.line()};
        auto it = lineView.begin();

        for (const auto& segment: parseResult.segments()){
          out << helpers::make_view(it, segment.view.begin())
              << segment;
          it = segment.view.end();
        }
        out << Mode::Normal << Color::Reset << helpers::make_view(it, lineView.end());
        break;
      }
    }

   // Assistive (parseResult)
    stringstream block;
    auto description = help.describe(parseResult);
    assist(block, parseResult.segments(), description);
    shell.output(block);
    _shell=&shell;
    _size = parseResult.segments().size();
  }
}

void Assistive::commandExecute(const Line& line) {
  if (!_active){
    return;
  }
  _shell->clear(_size);
}

bool Assistive::toggle(){
  return _active = !_active;
}
