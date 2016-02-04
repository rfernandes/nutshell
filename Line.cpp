#include "Line.h"

#include <algorithm>
#include <iterator>
#include <sstream>

using namespace std;

Line::Line()
: _segments(1)
, _active{_segments.begin()}
{
}

std::string Line::parameters() const {
  stringstream ss;
  copy(_segments.begin() + 1, _segments.end(), ostream_iterator<string>(ss, " "));
  string parameters = ss.str();
  if (_segments.size() > 1){
    parameters.resize(parameters.size() - 1);
  }
  return parameters;
}

bool Line::empty() const {
  return _active->empty() && _segments.size() == 1;
}

void Line::push(unsigned int letter){
  switch (letter) {
    case ' ':
      if (++_active == _segments.end()) {
        _segments.emplace_back();
        _active = _segments.begin() + _segments.size() - 1;
      }
      break;
    default:
      _active->push_back(letter);
  }
}

void Line::pop() {
  if (!_active->empty()) {
    _active->pop_back();
  } else if (_segments.size() > 1){
    _active = --_segments.erase(_active);
  }
}

const std::string & Line::command() const {
  return _segments.front();
}

unsigned int Line::width() const {
  return accumulate(_segments.begin(),
                    _segments.end(),
                    0u,
                    [](unsigned acc, const string& str){
                      return acc + str.size();
                    }) + _segments.size() - 1;
}
