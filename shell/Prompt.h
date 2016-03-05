#ifndef PROMPT_H
#define PROMPT_H

#include <string>

class Prompt
{
  const std::string _prompt;
public:
  Prompt();

  /// display the prompt
  void operator()() const;

  /// get width of the last line of the prompt
  unsigned width() const;
};

#endif
