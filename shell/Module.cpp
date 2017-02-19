#include "Module.h"

Module::~Module(){
}

void Module::commandExecute(const Line& /*line*/, Shell& /*shell*/){
}

void Module::commandExecuted(const ParseResult& /*parseResult*/, const Line& /*line*/, Shell& /*shell*/){
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
        if (dependency->initialize(*module)){
          --size;
        }
      }
    }
    prevSize = size;
  }
}

bool ModuleStore::keyPress(unsigned int keystroke, Shell& shell) {
  bool handledKey{false};
  for (auto &module: _modules){
    if (module->keyPress(keystroke, shell)) {
      handledKey = true;
      break;
    }
  }
  return handledKey;
}

void ModuleStore::lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell& shell) {
  for (auto &module: _modules){
    module->lineUpdated(parseResult, line, shell);
  }
}


void ModuleStore::commandExecute(const Line& line, Shell& shell) {
  for (auto &module: _modules){
    module->commandExecute(line, shell);
  }
}

void ModuleStore::commandExecuted(const ParseResult& parseResult, const Line& line, Shell& shell) {
  for (auto &module: _modules){
    module->commandExecuted(parseResult, line, shell);
  }
}
