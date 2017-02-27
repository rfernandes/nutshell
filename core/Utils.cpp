#include "Utils.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace utf8{

  //TODO: Move to Utils
  size_t size(const std::string& str){
    return count_if(str.begin(), str.end(), is_utf8);
  }

  /// Returns the utf8 codepoint of byte position idx
  size_t idx(const std::string& str, unsigned idx){
    return count_if(str.begin(), str.begin() + idx, is_utf8);
  }

}

namespace manip{
  repeat::repeat(char ch, size_t size)
  :_ch{ch}
  ,_size{size}
  {}

  ostream& operator << (ostream& out, const repeat& rep){
    fill_n(ostreambuf_iterator<char>(out), rep._size, rep._ch);
    return out;
  }
}

