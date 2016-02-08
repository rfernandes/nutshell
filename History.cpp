#include "History.h"

History::History()
: _idx{0}
{
}

void History::add(const Line& command) {
  _history.emplace_back(command);
  _idx = _history.size();
}

const Line& History::forward() {
  if (_idx < _history.size() - 1) {
    ++_idx;
  }
  return _history[_idx];
}

const Line& History::backward() {
  if (_idx > 0) {
    --_idx;
  }
  return _history[_idx];
}
