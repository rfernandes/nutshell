#ifndef PREDICTIVE_H
#define PREDICTIVE_H

#include <shell/Module.h>

class History;

class Predictive: public Module{
  bool _active;
  Suggestion _suggestion;
  History &_history;
public:
  Predictive(History& history);

  void lineUpdated(const Description& description, Shell &shell) override;
  void commandExecute(const Line &line, Shell &shell) override;
  void commandExecuted(const Description &description, Shell & shell) override;
  bool keyPress(unsigned int keystroke, Shell &shell) override;

  bool toggle();
};

#endif
