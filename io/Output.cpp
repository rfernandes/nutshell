#include "Output.h"

using namespace std;

namespace manip {

std::ostream& operator<< (std::ostream& out, const erase& manip) {
  return out << "\x1b[" << manip._eraseType << 'K';
}

std::ostream& operator<< (std::ostream& out, const color& manip) {
  if (manip._color == Reset) {
    out << "\x1b[39;49m";
  } else {
    out << "\x1b[3" << manip._color << 'm';
  }
  return out;
}

}
