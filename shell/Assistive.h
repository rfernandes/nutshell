#ifndef ASSISTIVE_H
#define ASSISTIVE_H

#include <shell/Module.h>

class Assistive: public Module{
  bool _active;

public:
  Assistive();

  void lineUpdated(const ParseResult& parseResult, Shell &shell) override;

  bool toggle();
};

#endif
