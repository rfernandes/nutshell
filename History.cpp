#include "History.h"

void History::add(const std::string& command)
{
  _history.emplace_back(command);
}

