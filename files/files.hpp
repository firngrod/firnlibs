#pragma once
#include <string>
#include <functional>

namespace FirnLibs
{
  namespace Files
  {
    // Cleans a linux path.  It removes double and trailing /s and evaluates relative paths.
    std::string CleanPath(const std::string &input);

    // Returns home path.
    inline std::string HomePath() { return std::string(getenv("HOME")); }

    // Checks if a file exists.
    inline bool Exists(const std::string filePath);

    // Iterate through files in a directory and perform a callback on each one.
    void ForEachFile(const std::string &root, const std::function<void (std::string &)> &callback, const bool &recursive = false);

    // Check if a file has a specific extension
    bool HasExtension(const std::string &fileName, const std::string &extension, const bool &caseSensitive = true);
  }
}
