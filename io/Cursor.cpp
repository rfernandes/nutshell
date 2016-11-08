#include "Cursor.h"

#include <iostream>

using namespace std;

Cursor::Cursor(std::istream& input, std::ostream& output)
: _in{input}
, _out{output}
{
}

void Cursor::up(unsigned rows) {
  _out << "\x1b[" << rows << 'A';
}

void Cursor::down(unsigned rows) {
  _out << "\x1b[" << rows << 'B';
}

void Cursor::forceDown()
{
  _out << "\x1b" << "D";
}

void Cursor::left(unsigned columns) {
  _out << "\x1b[" << columns << 'D';
}

void Cursor::right(unsigned columns) {
  _out << "\x1b[" << columns << 'C';
}

void Cursor::position(Cursor::Position position) {
  _out << "\x1b[" << position.y << ';' << position.x  << 'H';
}

void Cursor::save()
{
  _out << "\x1b[s";
}

void Cursor::restore()
{
  _out << "\x1b[u";
}

Cursor::Position Cursor::max() {
  auto start = this->position();
  this->position({999,999});
  auto maxPosition = this->position();
  this->position(start);
  return maxPosition;
}

Cursor::Position Cursor::position() const {
  _out << "\x1b[6n";
  _in.get(); // ^
  _in.get(); // [
  unsigned y;
  _in >> y;
  _in.get(); // ;
  unsigned x;
  _in >> x;
  _in.get(); // R
  return {x, y};
}

void Cursor::column(unsigned int column)
{
  _out << "\x1b[" << column << 'G'; // Non ANSI
}
