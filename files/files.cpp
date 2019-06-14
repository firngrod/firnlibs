#include "files.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <list>
#include <json/value.h>
#include <iostream>
#include <algorithm>
#ifdef _MSC_VER
#include <filesystem>
#else
#include <boost/filesystem.hpp>
#endif

namespace FirnLibs { namespace Files {


std::string CleanPath(const std::string &input)
{
  char temp[0x1000];
  char * ptr = temp;
  for(std::string::const_iterator itr = input.begin(), end = input.end(); itr != end && *itr != '\0'; itr++)
  {
    if(*itr == '/' && (((itr + 1) == end) || (*(itr + 1) == '/'))) // Remove double and trailing /
    {
      itr++;
      if(itr == end)
        break;
    }
    else
      *ptr++ = *itr;
  }
  *ptr = '\0';
  realpath(std::string(temp).c_str(), temp);
  return std::string(temp);
}


bool Exists(const std::string filePath)
{
  struct stat buffer;
  return (stat (filePath.c_str(), &buffer) == 0);
}

time_t FileModifiedTime(const std::string &filePath)
{
  struct stat buffer;
  if(stat(filePath.c_str(), &buffer) == 0)
  {
    return buffer.st_mtime;
  }
  return 0;
}


time_t FileAccessTime(const std::string &filePath)
{
  struct stat buffer;
  if(stat(filePath.c_str(), &buffer) == 0)
  {
    return buffer.st_atime;
  }
  return 0;
}


time_t FileStatusTime(const std::string &filePath)
{
  struct stat buffer;
  if(stat(filePath.c_str(), &buffer) == 0)
  {
    return buffer.st_ctime;
  }
  return 0;
}


void ForEachFile(const std::string &root, const std::function<void (std::string &)> &callback, const bool &recursive)
{
  // Initializations
  DIR *dir;
  struct dirent *ent;
  std::list<std::string> directoryQueue;

  // Push the root to the queue and start the scan loop
  directoryQueue.push_back(root);
  while(directoryQueue.size())
  {
    // Grab the front of the queue and erase it from the queue
    std::string thisDirectory = *directoryQueue.begin();
    directoryQueue.erase(directoryQueue.begin());
    if((dir = opendir(thisDirectory.c_str())) != nullptr) 
    {  // Try to open the directory
      while ((ent = readdir(dir)) != nullptr) 
      {
        // First, see if it is a directory.  If so, we add id to the queue for later scans if we are doing recursive
        if(recursive && (ent->d_type == 4 && *ent->d_name != '.'))
          directoryQueue.push_back(FirnLibs::Files::CleanPath(thisDirectory + "/" + ent->d_name));
        else
        {
          // If it is not a directory, it's a file, and so we run the callback.
          std::string fileName = FirnLibs::Files::CleanPath(thisDirectory + "/" + ent->d_name);
          callback(fileName);
        }
      }
      closedir(dir);
    }
  }
}


bool HasExtension(const std::string &fileName, const std::string &extension, const bool &caseSensitive)
{
  int lastDot = fileName.find_last_of('.');
  if(lastDot == std::string::npos)
    return false;
  int lastSlash = fileName.find_last_of('/');
  if(lastSlash > lastDot)
    return false;
  
  std::string fileExtension = fileName.substr(++lastDot, fileName.size() - lastDot);

  if(caseSensitive)
  {
    return extension == fileExtension;
  }

  std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), 
    [](const unsigned char &c) -> unsigned char { return std::toupper(c);});

  std::string tmpExt = extension;
  std::transform(tmpExt.begin(), tmpExt.end(), tmpExt.begin(),
    [](const unsigned char &c) -> unsigned char { return std::toupper(c);});

  return tmpExt == fileExtension;
}


bool CreateFolder(const std::string &dirPath)
{
  std::string cleanPath = CleanPath(dirPath);
  if(!Exists(cleanPath))
  {
    boost::filesystem::create_directory(cleanPath);
    if(!Exists(cleanPath))
    {
      return false;
    }
  }
  return true;
}


}}
