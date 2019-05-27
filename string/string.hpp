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

    // Transform a string to lower case
    inline void ToLower(std::string &str);

    // Transform a string to upper case
    inline void ToUpper(std::string &str);

    // Compare strings without case.
    bool CmpNoCase(const std::string &first, const std::string &second, const bool &noWaitIWannaDoCaseAnyway = false);

    // Replace string occurences in string with other string
    std::string Replace(const std::string &original, const std::string &oldstr, const std::string &newstr,
                        const bool &noCase = false, const size_t &start = 0, const size_t cnt = -1);
  }
}
