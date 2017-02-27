#include "Assistive.h"

#include <core/BuiltIn.h>
#include <core/Utils.h>
#include <io/Output.h>
#include <shell/Shell.h>
#include <shell/History.h>

#include <experimental/filesystem>
#include <experimental/string_view>
#include <unordered_map>

using namespace std;
using namespace std::experimental;
using namespace std::experimental::filesystem;
using namespace manip;

namespace {
  auto &assistive = ModuleStore::store<Assistive>();

  class HelpDatabase{
    unordered_map <string, std::pair<string, unordered_map<string, string>>> _db;
  public:

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
      }}}
    }
    {}

    vector<string> describe(const ParseResult& parseResult){
      vector<string> ret;

      const auto &segments = parseResult.segments();
      if (!segments.empty()){
        const auto &command = segments.front().view;

        const auto commandIt = _db.find(command.to_string());
        if (commandIt != _db.end()){
          ret.emplace_back(commandIt->second.first);
          for (const auto& segment: segments){
            // FIXME: Add string_view method to Segment
            if (&segment != &segments.front()){
              auto parameterIt = commandIt->second.second.find(segment.view.to_string());
              if (parameterIt != commandIt->second.second.end()){
                ret.emplace_back(parameterIt->second);
              }else{
                ret.emplace_back(string("unknown ") + enum_cast<const char *>(segment.type));
              }
            }
          }
        }else{
          for (const auto& segment: segments){
            ret.emplace_back(string("unknown ") + enum_cast<const char *>(segment.type));
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

  switch (parseResult.status()){
    case Status::NoMatch:
      out << Color::Red << line.line() << Color::Reset;
      break;
    default:{
      auto it = line.line().begin();

      for (const auto& segment: parseResult.segments()){
        while (it != segment.view.begin()){
          out << *it;
          ++it;
        }
        out << segment.type;
        while (it != segment.view.end()){
          out << *it;
          ++it;
        }
      }
      out << Mode::Normal << Color::Reset;
      while (it != line.line().end()){
        out << *it;
        ++it;
      }
      break;
    }
  }

   // Assistive (parseResult)
  if (_active){
    stringstream block;
    auto description = help.describe(parseResult);
    assist(block, parseResult.segments(), description);
    shell.output(block);
  }
}

bool Assistive::toggle(){
  return _active = !_active;
}
