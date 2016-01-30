#ifndef PROMPT_H
#define PROMPT_H

#include <string>

class Prompt
{
  const std::string _prompt;
public:
  Prompt();

  void operator()();
};

#endif