#ifndef PATH_H
#define PATH_H

#include <experimental/filesystem>

#include <string_view>

class Path{
  std::vector<std::experimental::filesystem::path> _path;
public:
  Path();
};

#endif
