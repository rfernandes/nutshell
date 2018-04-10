#ifndef ASSISTIVE_H
#define ASSISTIVE_H

#include <core/Module.h>

class Assistive: public Module{
  bool _active;

  int _size;
  Shell* _shell{nullptr};

public:
  Assistive();

  void lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell &shell) override;
  void commandExecute(const Line& line) override;

  bool toggle();
};

#endif
