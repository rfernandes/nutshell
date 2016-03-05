#ifndef LINE_H
#define LINE_H

#include <string>
#include <vector>

class Line
{
public:
  using Segments = std::vector<std::string>;

  Line();
  Line(std::initializer_list<std::string> segmengs);

  const std::string& command() const;
  const Segments& segments() const;


  std::string parameters() const;

  std::string operator()() const;

  void push(unsigned letter);
  void pop();

  size_t parameterCount() const;

  bool empty() const;
  unsigned width() const;

private:
  Segments _segments;
  Segments::iterator _active;
};

#endif
