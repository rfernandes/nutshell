#include "Cursor.h"

#include <iostream>

using namespace std;

void Cursor::up(unsigned rows) {
  cout << "\x1b[" << rows << 'A';
}

void Cursor::down(unsigned rows) {
  cout << "\x1b[" << rows << 'B';
}

void Cursor::left(unsigned columns) {
  cout << "\x1b[" << columns << 'D';
}

void Cursor::right(unsigned columns) {
  cout << "\x1b[" << columns << 'C';
}

void Cursor::position(Cursor::Position position) {
  cout << "\x1b[" << position.y << ';' << position.x  << 'H';
}

Cursor::Position Cursor::max() {
  auto start = this->position();
  this->position({999,999});
  auto maxPosition = this->position();
  this->position(start);
  return maxPosition;
}

Cursor::Position Cursor::position() const {
  cout << "\x1b[6n";
  cin.get(); // ^
  cin.get(); // [
  unsigned y;
  cin >> y;
  cin.get(); // ;
  unsigned x;
  cin >> x;
  cin.get(); // R
  return {x, y};
}

void Cursor::column(unsigned int column)
{
  cout << "\x1b[" << column << 'G'; // Non ANSI
}
