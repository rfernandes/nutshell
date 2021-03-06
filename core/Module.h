#ifndef MODULE_H
#define MODULE_H

#include <memory>
#include <vector>

class Shell;
class ParseResult;
class LineBuffer;
using Line = std::string;

class Module{
public:
  virtual ~Module() = 0;
  virtual bool keyPress(unsigned keystroke, Shell& shell);
  virtual void lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell& shell);
  virtual void commandExecute(const Line& line);
  virtual void commandExecuted(const ParseResult& parseResult, const Line& line);
};

class Dependency
{
public:
  virtual ~Dependency(){}

  virtual bool initialize(Module &module) = 0;
};

template<typename ModuleT, typename DependencyT>
class ModuleDependency: public Dependency{
public:
  bool initialize(Module &module) override;
};

template <typename TypeT>
struct Priority{
  short priority{10};
  std::unique_ptr<TypeT> instance;

  bool operator < (const Priority<TypeT>& other) const{
    return priority < other.priority;
  }
};

class ModuleStore: public Module{
public:

  template<typename ModuleT, short Priority = 10, typename ...Args>
  static ModuleT& store(Args&& ...args) {
    auto &modules = instance()._modules;
    modules.push_back({Priority, std::make_unique<ModuleT>(std::forward<Args>(args)...)});
    return *static_cast<ModuleT*>(modules.back().instance.get());
  }

  template<typename ModuleT, typename DependencyT, typename ...Args>
  static ModuleDependency<ModuleT, DependencyT>& dependency(Args&& ...args) {
    auto &_dependency = instance()._dependency;
    _dependency.push_back(std::make_unique<ModuleDependency<ModuleT, DependencyT>>(std::forward<Args>(args)...));
    return *static_cast<ModuleDependency<ModuleT, DependencyT>*>(_dependency.back().get());
  }

  using Modules = std::vector<Priority<Module>>;

  void initialize();

  bool keyPress(unsigned int keystroke, Shell & shell) override;
  void lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell & shell) override;
  void commandExecute(const Line & line) override;
  void commandExecuted(const ParseResult & parseResult, const Line& line) override;

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
