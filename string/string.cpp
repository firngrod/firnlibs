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
  vsnprintf(Buffer, sizeof(Buffer), Format.c_str(), FormatArgs);
  output = Buffer;
  va_end(FormatArgs);
  delete[] Buffer;
}


namespace FirnLibs{ namespace String{ 


#ifdef _MSC_VER
void StringPrintf(std::string &output, const std::string format, ...)
#else
void StringPrintf(std::string &output, const std::string &format, ...)
#endif
{
  va_list formatArgs;
  va_start(formatArgs, format);
  StringPrintfVA(output, format, formatArgs);
}


#ifdef _MSC_VER
std::string StringPrintf(const std::string format, ...)
#else
std::string StringPrintf(const std::string &format, ...)
#endif
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

#ifdef _MSC_VER
std::string WideStringToString(const std::wstring &str, const int &codePage)
{
	size_t bufSize = WideCharToMultiByte(codePage, 0, str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);
	char * tmpForOut = new char[bufSize + 1];
	WideCharToMultiByte(codePage, 0, str.c_str(), str.size(), tmpForOut, sizeof(tmpForOut), nullptr, nullptr);
	tmpForOut[sizeof(tmpForOut) - 1] = '\0';
	std::string strOut = tmpForOut;
	delete tmpForOut;
	return strOut;
}

std::wstring StringToWideString(const std::string &str, const int &codePage)
{
	size_t bufSize = MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), nullptr, 0);
	wchar_t *tmpForOut = new wchar_t[bufSize + 1];
	MultiByteToWideChar(codePage, 0, str.c_str(), str.size(), tmpForOut, sizeof(tmpForOut));
	tmpForOut[sizeof(tmpForOut) - 1] = L'\0';
	std::wstring strOut = tmpForOut;
	delete tmpForOut;
	return strOut;
}

#endif
}}
