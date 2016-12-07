#include "LineBuffer.h"

#include <io/Input.h>
#include <shell/Shell.h>
#include <shell/Utils.h>

#include <algorithm>

using namespace std;
using namespace std::experimental;

namespace {
  auto lineBuffer = ModuleStore::store<LineBuffer>();
}

LineBuffer::LineBuffer() = default;

LineBuffer::LineBuffer(std::string line)
: _line{std::move(line)}
{
}

void LineBuffer::backwardKillWord(){
  if (_idx){
    auto start = _idx;
    while (--_idx && _line[_idx] == ' '){};
    while (_idx-- && _line[_idx] != ' '){};
    if (_idx != 0) ++_idx;
    _line.erase(_idx, start - _idx);
  }
}

const std::string& LineBuffer::line() const{
  return _line;
}

string_view LineBuffer::firstWord() const{
  return string_view{_line};
}

void LineBuffer::line(const std::string& line){
  _line = line;
  _idx = utf8::size(_line);
}

void LineBuffer::clear(){
  _idx = 0;
  _line.clear();
}

void LineBuffer::insert(unsigned int ch){
  _line.insert(_idx++, 1, ch);
}

std::size_t LineBuffer::idx() const{
  return _idx;
}

std::size_t LineBuffer::pos() const{
 return utf8::idx(_line, _idx);;
}

bool LineBuffer::keyPress(unsigned int keystroke, Shell& shell){
  switch (keystroke){
    case 23: //Ctrl-W
      backwardKillWord();
      break;
    case Input::Backspace:
    case '\b': // Ctrl-H
      if (!_idx){
        break;
      }
      while (!utf8::is_utf8(_line[--_idx])){};
    case Input::Delete:
      _line.erase(_idx, utf8::bytes(_line[_idx]));
      break;
    case Input::Left:
      if (_idx > 0){
        while (!utf8::is_utf8(_line[--_idx])){}
      }
      break;
    case Input::Right:
      if (_line.size() > _idx){
        _idx += utf8::bytes(_line[_idx]);;
      }
      break;
    case Input::Home:
      _idx = 0;
      break;
    case Input::End:
      _idx = _line.size();
      break;
    case '\n':
      shell.executeCommand(*this);
      clear();
      break;
    case 4: //Ctrl-D
      shell.exit();
      return false;
    case Input::Unknown: // Unknown special key received
      return false; // ignore list
    default:
      insert(keystroke);
      break;
  }
  shell.displayLine(*this);
  return true;
}