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

const std::string & Line::command() const {
  return _segments.front();
}

std::string Line::parameters() const {
  string ret;
  if (_segments.size() > 1) {
    stringstream ss;
    copy(_segments.begin() + 1, _segments.end(), ostream_iterator<string>(ss, " "));
    ret = ss.str();
    ret.resize(ret.size() - 1);
  }
  return ret;
}

std::string Line::operator()() const{
  stringstream ss;
  copy(_segments.begin(), _segments.end(), ostream_iterator<string>(ss, " "));
  string line = ss.str();
  line.resize(line.size() - 1);
  return line;
}

bool Line::empty() const {
  return _active->empty() && _segments.size() == 1;
}

size_t Line::parameterCount() const {
  return _segments.size() - 1;
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

unsigned int Line::width() const {
  return accumulate(_segments.begin(),
                    _segments.end(),
                    0u,
                    [](unsigned acc, const string& str){
                      return acc + str.size();
                    }) + _segments.size() - 1;
}
