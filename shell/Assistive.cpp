#include "Assistive.h"

#include <command/BuiltIn.h>
#include <io/Output.h>
#include <shell/History.h>

#include <experimental/string_view>

using namespace std;
using namespace std::experimental;
using namespace manip;

namespace {
  auto &assistive = ModuleStore::store<Assistive>();

  /* TODO:
  *    Cleanup single suggestion when deleting last match
  *    Clear &/Pad when executed
  *    Compact view (maximise line information)
  */
  ostream & assist(ostream &out, const Line::const_iterator &start, const Description &description, size_t idx){
    auto it = start;
    for (size_t i = 0; i < idx; ++i) {
      const auto &segment = description.segments().at(i);
      fill_n(ostreambuf_iterator<char>(out), distance(it, segment.begin), ' ');
      it = segment.begin;
      if (i + 1 == idx){
        switch (segment.type){
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
          default:
            out << Color::Yellow;
        }
        out << "╰╸last";
      }else{
        out << "│" << Color::Reset;
        ++it;
      }
    }

    if (idx > 0){
      out << "\n";
      assist(out, start, description, idx - 1);
    }
    return out;
  }
}

Assistive::Assistive()
:_active{false}
{
  // Toggle predictive functionality
  CommandStore::store<BuiltIn>(":assistive",
                              [=](const Line& /*line*/, Output& /*output*/){
                                toggle();
                                return Status::Ok;
                              });
}

void Assistive::lineUpdated(const Description& description, Shell& shell){
  auto& out = shell.out();

  switch (description.status()){
    case Status::NoMatch:
      out << Color::Red << shell.line() << Color::Reset;
      break;
    default:{
      auto it = shell.line().begin();

      for (const auto& segment: description.segments()){
        while (it != segment.begin){
          out << *it;
          ++it;
        }
        switch (segment.type){
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
          default:
            out << Color::Yellow;
        }
        while (it != segment.end){
          out << *it;
          ++it;
        }
      }
      out << Mode::Normal << Color::Reset;
      while (it != shell.line().end()){
        out << *it;
        ++it;
      }
      break;
    }
  }

   // Assistive (description)
  if (_active){
    auto segments = description.segments().size();
    stringstream block;

    assist(block, shell.line().begin(), description, segments);

    shell.output(block);
  }
}

bool Assistive::toggle(){
  return _active = !_active;
}
