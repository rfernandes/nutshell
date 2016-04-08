#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>

namespace manip {

enum class Erase: uint8_t { CursorToEnd, CursorToBegin, All};
std::ostream& operator<< (std::ostream& out, Erase eraseType);

enum class Color: uint8_t {Black, Red, Green, Yellow, Blue, Magenta, Cyan, White, Reset};
std::ostream& operator<< (std::ostream& out, Color color);

class rgb{
  uint8_t _red,_green,_blue;

public:
  rgb(uint8_t red, uint8_t green, uint8_t blue);

  friend std::ostream& operator<< (std::ostream& out, const rgb& manip);
};

}

using Output = std::ostream;

#endif
