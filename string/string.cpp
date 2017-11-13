#include "string.hpp"

#include <cstdarg>
#include <algorithm>

void StringPrintfVA(std::string &output, const std::string &Format, va_list &FormatArgs)
{
  // First, find the length of the formatted string.  This partly to limit RAM use, but mostly to make absolutely
  // sure that no albeit huge predefined buffer will ever overflow.
  // I read somewhere that vsnprintf can modify the arglist, so we do a copy for it to play with.
  va_list FormatArguments;
  va_copy(FormatArguments, FormatArgs);
  int TotalLength = vsnprintf(nullptr, 0, Format.c_str(), FormatArguments);
  va_end(FormatArguments);
  // Now use this info to make a buffer and vsprintf into it, convert it into an std::string, clean up and return
  // the string.
  char * Buffer = new char[TotalLength + 1];
  vsprintf(Buffer, Format.c_str(), FormatArgs);
  output = Buffer;
  va_end(FormatArgs);
  delete[] Buffer;
}


namespace FirnLibs{ namespace String{ 



void StringPrintf(std::string &output, const std::string &format, ...)
{
  va_list formatArgs;
  va_start(formatArgs, format);
  StringPrintfVA(output, format, formatArgs);
}

std::string StringPrintf(const std::string &format, ...)
{
  va_list formatArgs;
  va_start(formatArgs, format);
  std::string returnString;
  StringPrintfVA(returnString, format, formatArgs);
  return returnString;
}

std::vector<std::string> Split(const std::string &input, const std::string &delimiters, const bool &noEmpty)
{
  std::vector<std::string> output;
  size_t pos = 0, nextDelim;
  while(pos < input.size())
  {
    nextDelim = input.find_first_of(delimiters, pos);
    if(pos == nextDelim && noEmpty)
    {
    }
    else if(nextDelim == std::string::npos)
    {
      output.push_back(input.substr(pos, input.size() - pos));
      break;
    }
    else
    {
      output.push_back(input.substr(pos, nextDelim - pos));
    }
    pos = nextDelim + 1;
  }
  return output;
}

// I have the option to not ignore case in case it's a user input or something whether to do case.
bool CmpNoCase(const std::string &first, const std::string &second, const bool &noWaitIWannaDoCaseAnyway)
{
  if(noWaitIWannaDoCaseAnyway)
    return first == second;

  std::string lfirst = first;
  std::string lsecond = second;
  std::transform(lfirst.begin(), lfirst.end(), lfirst.begin(), ::tolower);
  std::transform(lsecond.begin(), lsecond.end(), lsecond.begin(), ::tolower);
  return lfirst == lsecond;
}

}}
