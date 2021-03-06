#include "Module.h"

#include <algorithm>

using namespace std;

Module::~Module(){
}

void Module::commandExecute(const Line& /*line*/){
}

void Module::commandExecuted(const ParseResult& /*parseResult*/, const Line& /*line*/){
}

bool Module::keyPress(unsigned int /*keystroke*/, Shell& /*shell*/){
  return false;
}

void Module::lineUpdated(const ParseResult& /*parseResult*/, const LineBuffer& /*line*/, Shell& /*shell*/){
}


ModuleStore& ModuleStore::instance(){
  static ModuleStore store;
  return store;
}

// FIXME: call once
void ModuleStore::initialize(){
  auto size{_dependency.size()};
  auto prevSize{0u};
  while (size && prevSize != size){
    //FIXME: This will loop forever on double-dependencies
    for (auto& dependency: _dependency){
      for (auto& module: _modules){
        if (dependency->initialize(*module.instance)){
          --size;
        }
      }
    }
    prevSize = size;
  }
  sort(begin(_modules), end(_modules));
}

bool ModuleStore::keyPress(unsigned int keystroke, Shell& shell) {
  bool handledKey{false};
  for (auto &module: _modules){
    if (module.instance->keyPress(keystroke, shell)) {
      handledKey = true;
      break;
    }
  }
  return handledKey;
}

void ModuleStore::lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell& shell) {
  for (auto &module: _modules){
    module.instance->lineUpdated(parseResult, line, shell);
  }
}


void ModuleStore::commandExecute(const Line& line) {
  for (auto &module: _modules){
    module.instance->commandExecute(line);
  }
}

void ModuleStore::commandExecuted(const ParseResult& parseResult, const Line& line) {
  for (auto &module: _modules){
    module.instance->commandExecuted(parseResult, line);
  }
}
