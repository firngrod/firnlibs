#pragma once
#include <string>
#include <functional>
#include <vector>

namespace FirnLibs
{
  namespace Files
  {
    // Error codes
    enum ErrorCode
    {
      None,
      FileDoesNotExist,
    };

    // Cleans a linux path.  It removes double and trailing /s and evaluates relative paths.
    std::string CleanPath(const std::string &input);

    // Returns home path.
    inline std::string HomePath() { return std::string(getenv("HOME")); }

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
    
    // Read a file into binary buffer.
    ErrorCode ReadFile(std::vector<unsigned char> &outBuffer, const std::string &filePath);

  }
}
