#ifndef HISTORY
#define HISTORY

#include <vector>
#include <string>


class History
{
  std::vector<std::string> _history;
  std::size_t _idx;
public:
  void add(const std::string& command);

  void forward()
  {
  }

  void backward()
  {
  }
};


#endif
