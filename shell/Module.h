#ifndef MODULE_H
#define MODULE_H

#include <shell/Shell.h>

#include <memory>
#include <vector>

class Module{
public:
  virtual ~Module() = 0;
  virtual bool keyPress(unsigned keystroke, Shell& shell);
  virtual void lineUpdated(const Description& description, Shell& shell);
  virtual void commandExecute(const Line& line, Shell& shell);
  virtual void commandExecuted(const Description& description, Shell& shell);
};

class Dependency
{
public:
  virtual bool initialize(Module &module) = 0;
};

template<typename ModuleT, typename DependencyT>
class ModuleDependency: public Dependency{
public:
  bool initialize(Module &module) override;
};

class ModuleStore{
public:

  template<typename ModuleT, typename ...Args>
  static ModuleT& store(Args&& ...args) {
    auto &modules = instance()._modules;
    modules.push_back(std::make_unique<ModuleT>(std::forward<Args>(args)...));
    return *static_cast<ModuleT*>(modules.back().get());
  }

  template<typename ModuleT, typename DependencyT, typename ...Args>
  static ModuleDependency<ModuleT, DependencyT>& dependency(Args&& ...args) {
    auto &_dependency = instance()._dependency;
    _dependency.push_back(std::make_unique<ModuleDependency<ModuleT, DependencyT>>(std::forward<Args>(args)...));
    return *static_cast<ModuleDependency<ModuleT, DependencyT>*>(_dependency.back().get());
  }

  using Modules = std::vector<std::unique_ptr<Module>>;

  static const Modules& modules();

  void initialize();
private:
  static ModuleStore& instance();
  std::vector<std::unique_ptr<Dependency>> _dependency;
  Modules _modules;

  friend class Shell;
};

template<typename ModuleT, typename DependencyT>
bool ModuleDependency<ModuleT, DependencyT>::initialize(Module& module)
{
  DependencyT* cast = dynamic_cast<DependencyT*>(&module);
  if (cast){
    ModuleStore::store<ModuleT>(*cast);
  }
  return cast;
}


#endif
