#include "Prompt.h"

#include <iostream>

using namespace std;

Prompt::Prompt()
: _prompt{"NutShell 0.1α >"}
{
}

void Prompt::operator()() const {
  cout << "\x1b[38;2;150;150;230m" << _prompt.data() << "\x1b[0m";
}

unsigned Prompt::width() const{
  return _prompt.size(); // FIXME: Only support one line prompts;
}
