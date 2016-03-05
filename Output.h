#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include <type_traits>
#include <string>

namespace manip {

enum EraseType: char { CursorToEnd, CursorToBegin, All};

class erase{
  EraseType _eraseType;

public:
  explicit constexpr erase(EraseType eraseType) : _eraseType{eraseType} {}

  friend std::ostream& operator<< (std::ostream& out, const erase& manip );
};

enum Color: char {Black, Red, Green, Yellow, Blue, Magenta, Cyan, White, Reset};

struct color{
  Color _color;

public:
  explicit color(Color color): _color{color} {}

  friend std::ostream& operator<< (std::ostream& out, const color& manip );
};

}

using Output = std::ostream;

#endif
