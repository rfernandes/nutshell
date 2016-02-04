#ifndef LINE_H
#define LINE_H

#include <string>
#include <vector>

class Line
{
  using Segments = std::vector<std::string>;

  Segments _segments;
  Segments::iterator _active;

public:

  Line();

  const std::string& command() const;
  std::string parameters() const;

  void push(unsigned letter);
  void pop();

  bool empty() const;
  unsigned width() const;
};

#endif