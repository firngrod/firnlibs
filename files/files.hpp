#pragma once
#include <string>
#include <functional>
#ifdef _MSC_VER
#include "Windows.h"
#include "../string/string.hpp"
#endif

namespace FirnLibs
{
  namespace Files
  {
    // Cleans a linux path.  It removes double and trailing /s and evaluates relative paths.
    std::string CleanPath(const std::string &input);

    // Returns home path.
	inline std::string HomePath()
	{
#ifdef _MSC_VER
		// Isn't the Windows way just wonderful compared to the Linux way?
		size_t bufSize = 0;
		wchar_t *output;
		errno_t rc = _wdupenv_s(&output, &bufSize, L"HOMEDRIVE");
		if (rc)
			return "";
		std::wstring wstrOut = output;
		free(output);
		rc = _wdupenv_s(&output, &bufSize, L"HOMEPATH");
		if (rc)
			return "";
		wstrOut += output;
		free(output);

		return FirnLibs::String::WideStringToString(wstrOut);
#else
		return std::string(getenv("HOME"));
#endif
	}

    // Checks if a file exists.
    bool Exists(const std::string filePath);

    // Get file timestamps
    time_t FileModifiedTime(const std::string &filePath);
    time_t FileAccessTime(const std::string &filePath);
    time_t FileStatusTime(const std::string &filePath);

    // Iterate through files in a directory and perform a callback on each one.
    void ForEachFile(const std::string &root, const std::function<void (std::string &)> &callback, const bool &recursive = false);

    // Check if a file has a specific extension
    bool HasExtension(const std::string &fileName, const std::string &extension, const bool &caseSensitive = true);

    // Create a folder
    bool CreateFolder(const std::string &dirPath);

  }
}
