#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <string>
#include <type_traits>

namespace utf8{

  constexpr bool is_utf8(char byte) noexcept{
    return (byte & 0xc0) != 0x80;
  }

  constexpr unsigned short bytes(char ch) noexcept{
    return (ch & 0b100'00000) == 0b0000'0000 ? 1:
           (ch & 0b111'00000) == 0b1100'0000 ? 2:
           (ch & 0b111'10000) == 0b1110'0000 ? 3:
           (ch & 0b111'11000) == 0b1111'0000 ? 4: 0;
  }

  size_t size(const std::string& str);
  size_t idx(const std::string& str, unsigned idx);
}

namespace manip{

  class repeat{
    char _ch;
    size_t _size;

  public:
    repeat(char ch, size_t size);

    friend std::ostream& operator<< (std::ostream& out, const repeat& manip);
  };
}

template <typename Enum>
constexpr const char* enum_data[] {""};

template <typename Data, typename Enum,
          typename std::enable_if<std::is_enum<Enum>::value>* = nullptr>
Data enum_cast(Enum from){
  return enum_data<Enum>[static_cast<std::underlying_type_t<Enum>>(from)];
}

template <typename Data, typename Enum,
          typename std::enable_if<std::is_enum<Enum>::value>* = nullptr>
Enum enum_cast(const Data& from){
  const auto& data{enum_data<Enum>};
  const auto idx = std::find(std::begin(data), std::end(data), from);
  return static_cast<Enum>(idx != std::end(data) ? idx: 0);
}

#endif
