#ifndef PREDICTIVE_H
#define PREDICTIVE_H

#include <shell/Line.h>
#include <shell/Module.h>

class History;

class Predictive: public Module{
  bool _active;
  Suggestion _suggestion;
  History &_history;
public:
  explicit Predictive(History& history);

  void lineUpdated(const ParseResult& parseResult, const LineBuffer& line, Shell &shell) override;
  bool keyPress(unsigned int keystroke, Shell &shell) override;

  bool toggle();
};

#endif
