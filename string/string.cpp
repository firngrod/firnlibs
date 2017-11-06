#include "string.hpp"

#include <cstdarg>

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



}}
