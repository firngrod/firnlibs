#pragma once
#include <string>

namespace FirnLibs
{
  namespace String
  {
    // Good ole sprintf, just with std::string.
    std::string StringPrintf(const std::string &format, ...);
    void StringPrintf(std::string output, const std::string &format, ...);
  }
}
