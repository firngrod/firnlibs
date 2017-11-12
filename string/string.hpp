#pragma once
#include <string>
#include <vector>

namespace FirnLibs
{
  namespace String
  {
    // Good ole sprintf, just with std::string.
    std::string StringPrintf(const std::string &format, ...);
    void StringPrintf(std::string output, const std::string &format, ...);

    // Split a string into tokens based on delimiters.
    std::vector<std::string> Split(const std::string &input, const std::string &delimiters = " ", const bool &noEmpty = true);

    // Compare strings without case.
    bool CmpNoCase(const std::string &first, const std::string &second, const bool &noWaitIWannaDoCaseAnyway = false);
  }
}
