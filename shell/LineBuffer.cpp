#include "LineBuffer.h"

#include <core/Utils.h>
#include <io/Input.h>
#include <shell/Shell.h>

#include <algorithm>

using namespace std;
using namespace std::experimental;

namespace {
  // Lower priority since LineBuffer consumes all non parsed keyPresses
  auto lineBuffer = ModuleStore::store<LineBuffer, 20>();
}

LineBuffer::LineBuffer() = default;

LineBuffer::LineBuffer(std::string line)
: _line{std::move(line)}
, _idx{utf8::size(_line)}
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

void LineBuffer::backwardChar(){
  if (_idx > 0){
    while (!utf8::is_utf8(_line[--_idx])){}
  }
}

void LineBuffer::forwardChar(){
  if (_line.size() > _idx){
    _idx += utf8::bytes(_line[_idx]);;
  }
}

void LineBuffer::backwardDeleteChar(){
  if (!_idx){
    return;
  }
  while (!utf8::is_utf8(_line[--_idx])){};
  deleteChar();
}

void LineBuffer::deleteChar(){
  _line.erase(_idx, utf8::bytes(_line[_idx]));
}

void LineBuffer::begginingOfLine(){
  _idx = 0;
}

void LineBuffer::endOfLine(){
  _idx = _line.size();
}

void LineBuffer::acceptLine(Shell& shell){
  shell.executeCommand(*this);
  clear();
}

const std::string& LineBuffer::line() const{
  return _line;
}

string_view LineBuffer::firstWord() const{
  const auto idx = _line.find_first_of(' ');
  return string_view{_line.data(), idx == string::npos ? _line.size(): idx};
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
      backwardDeleteChar();
      break;
    case Input::Delete:
      deleteChar();
      break;
    case Input::Left:
      backwardChar();
      break;
    case Input::Right:
      forwardChar();
      break;
    case Input::Home:
      begginingOfLine();
      break;
    case Input::End:
      endOfLine();
      break;
    case '\n':
      acceptLine(shell);
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

// Clean up
void LineBuffer::commandExecuted(const ParseResult & parseResult, const Line & /*line*/)
{
  switch (parseResult.status()) {
    case Status::Incomplete:
      _line += '\n';
      break;
    default:
      clear();
      break;
  }
}

void LineBuffer::lineUpdated(const ParseResult& /*parseResult*/, const LineBuffer& line, Shell& /*shell*/)
{
  _line = line._line;
  _idx = line._idx;
}

