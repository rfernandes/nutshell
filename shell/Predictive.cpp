#include "Predictive.h"

#include <core/BuiltIn.h>
#include <io/Output.h>
#include <shell/History.h>
#include <shell/Shell.h>

using namespace std;
using namespace manip;

namespace {
  auto &predictiveDep = ModuleStore::dependency<Predictive, History>();
}

Predictive::Predictive(History& history)
:_active{true}
,_history{history}
{
  // Toggle predictive functionality
  CommandStore::store<BuiltIn>(":predictive",
                               "Toggle history based prediction",
                              [=](const Line& /*line*/, Output& /*output*/){
                                toggle();
                                return Status::Ok;
                              });
}

void Predictive::lineUpdated(const ParseResult& /*parseResult*/, const LineBuffer& line, Shell& shell){
  auto &out = shell.out();

  if (_active){
     _suggestion = _history.suggest(line.line());
    if (!_suggestion.empty()){
      out << _suggestion.substr(line.line().size()) << Color::Reset;
    }
    out << Erase::CursorToEnd;
  }
}

bool Predictive::keyPress(unsigned int keystroke, Shell& shell){
  switch (keystroke){
    case Input::CtrlM:
      if (!_suggestion.empty()){
        shell.executeCommand(LineBuffer{_suggestion.to_string()});
        _suggestion = Suggestion{};
      }
      return true;
  }
  return false;
}

bool Predictive::toggle(){
  return _active = !_active;
}
