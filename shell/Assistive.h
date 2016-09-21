#ifndef ASSISTIVE_H
#define ASSISTIVE_H

#include <shell/Module.h>

class Assistive: public Module{
  bool _active;
public:
  Assistive();

  void lineUpdated(const Description& description, Shell &shell) override;
  void commandExecute(const Line &line, Shell &shell) override;
  void commandExecuted(const Description &description, Shell & shell) override;
  bool keyPress(unsigned int keystroke, Shell &shell) override;

  bool toggle();
};

#endif
