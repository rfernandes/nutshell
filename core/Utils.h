#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <cstring>
#include <string>
#include <type_traits>

namespace helpers{
  std::string_view make_view(std::string::const_iterator begin,
                             std::string::const_iterator end);
  std::string_view make_view(std::string_view::const_iterator begin,
                             std::string_view::const_iterator end);
}

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
constexpr const char* enum_data[] {nullptr};

template <typename Data, typename Enum,
          typename std::enable_if<std::is_enum<Enum>::value>::type* = nullptr>
constexpr Data enum_cast(Enum from){
  static_assert(enum_data<Enum>[0], "Missing specialization of enum_data for Enum type");
  return enum_data<Enum>[static_cast<std::underlying_type_t<Enum>>(from)];
}

template <typename Enum, typename Data,
          typename std::enable_if<std::is_enum<Enum>::value>::type* = nullptr>
Enum enum_cast(const Data& from){
  static_assert(enum_data<Enum>[0], "Missing specialization of enum_data for Enum type");
  const auto& data{enum_data<Enum>};
  const auto idx = std::find_if(std::begin(data), std::end(data), [from](auto data){
    return !std::strcmp(data, from);
  });
  return static_cast<Enum>(idx ? std::distance(std::begin(data), idx): 0);
}

#endif
