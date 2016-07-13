#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace utf8{

  constexpr bool is_utf8(char byte) noexcept{
    return (byte & 0xc0) != 0x80;
  }

  constexpr short bytes(char ch) noexcept{
    return (ch & 0b100'00000) == 0b0000'0000 ? 1:
           (ch & 0b111'00000) == 0b1100'0000 ? 2:
           (ch & 0b111'10000) == 0b1110'0000 ? 3:
           (ch & 0b111'11000) == 0b1111'0000 ? 4: 0;
  }

  size_t size(const std::string& str);
  size_t idx(const std::string& str, unsigned idx);
}

#endif
