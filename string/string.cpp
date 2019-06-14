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

void ToLower(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void ToUpper(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

// I have the option to not ignore case in case it's a user input or something whether to do case.
bool CmpNoCase(const std::string &first, const std::string &second, const bool &noWaitIWannaDoCaseAnyway)
{
  if(noWaitIWannaDoCaseAnyway)
    return first == second;

  std::string lfirst = first;
  std::string lsecond = second;
  ToLower(lfirst);
  ToLower(lsecond);
  return lfirst == lsecond;
}

std::string Replace(const std::string &original, const std::string &oldstr, const std::string &newstr, const bool &noCase, const size_t &start, const size_t cnt)
{
  // Sanity check
  if(cnt == 0 || oldstr.size() == 0 || start >= original.size() || CmpNoCase(newstr, oldstr, !noCase))
    return original;

  std::string org = original;
  std::string olds = oldstr;
  if(noCase)
  {
    ToLower(org);
    ToLower(olds);
  }

  std::string out = original.substr(0, start);
  size_t pos = 0, oldpos = start, cntDone = 0;
  while(pos != std::string::npos && cntDone < cnt)
  {
    pos = org.find(olds, oldpos);
    if(pos == std::string::npos)
    {
      out += original.substr(oldpos, original.size() - oldpos);
    }
    else
    {
      out += original.substr(oldpos, pos - oldpos);
      out += newstr;
      oldpos = pos + oldstr.size();
      ++cntDone;
      if(cntDone == cnt)
      {
        out += original.substr(oldpos, original.size() - oldpos);
      }
    }
  }
  return out;
}

std::string Trim(const std::string &original, const std::string &charsToTrim, const bool &trimLeft, const bool &trimRight)
{
  if(charsToTrim == "")
    return original;

  size_t start = trimLeft ? original.find_first_not_of(charsToTrim) : 0;
  if(start == std::string::npos)
    return "";
  size_t end = trimRight ? original.find_last_not_of(charsToTrim) : original.size();

  return original.substr(start, end - start + 1);
}

}}
