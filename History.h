#ifndef HISTORY
#define HISTORY

#include "Line.h"

class History
{
  std::vector<Line> _history;
  std::size_t _idx;

public:

  History();

  void add(const Line& command);
  const Line& forward();
  const Line& backward();
};


#endif