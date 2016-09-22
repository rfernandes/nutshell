#include "Predictive.h"

#include <command/BuiltIn.h>
#include <io/Output.h>
#include <shell/History.h>

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
                              [=](const Line& /*line*/, Output& /*output*/){
                                toggle();
                                return Status::Ok;
                              });
}

void Predictive::commandExecute(const Line& /*line*/, Shell& /*shell*/){
}

void Predictive::commandExecuted(const Description& /*description*/, Shell& /*shell*/){
}

void Predictive::lineUpdated(const Description& /*description*/, Shell& shell){
  if (_active){
     _suggestion = _history.suggest(shell.line());
    if (!_suggestion.empty()){
      //FIXME : call out on shell, instead of using cout
      cout << _suggestion.substr(shell.line().size()) << Color::Reset;
    }
    cout << Erase::CursorToEnd;
  }
}

bool Predictive::keyPress(unsigned int keystroke, Shell& shell){
  switch (keystroke){
    case Input::CtrlM:
      if (!_suggestion.empty()){
        shell.executeCommand(_suggestion.to_string());
        _suggestion = Suggestion{};
      }
      return true;
  }
  return false;
}

bool Predictive::toggle(){
  return _active = !_active;
}
