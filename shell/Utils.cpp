#include "Utils.h"

#include <algorithm>

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
