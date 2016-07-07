#include "Output.h"

#include <type_traits>

using namespace std;

bool isBold(manip::Color color){
  return color >= manip::Color::BoldBlack && color < manip::Color::Reset;
}

namespace manip {

std::ostream& operator<< (std::ostream& out, Erase eraseType) {
  return out << "\x1b[" << +static_cast<std::underlying_type_t<Color>>(eraseType) << 'K';
}

std::ostream& operator<< (std::ostream& out, Color color) {
  if (color == Color::Reset) {
    out << "\x1b[39;49m";
  } else {
    auto colorValue = static_cast<std::underlying_type_t<Color>>(color) & 0b111;

    out << "\x1b[" << (isBold(color) ? "1;3" : "3") << +colorValue << 'm';
  }
  return out;
}

rgb::rgb(uint8_t red, uint8_t green, uint8_t blue)
: _red{red}
, _green{green}
, _blue{blue}
{
}

std::ostream& operator<< (std::ostream& out, const rgb& manip) {
  return out << "\x1b[38;2;" << +manip._red << ';' << +manip._green << ';' << +manip._blue << 'm';
}

}
