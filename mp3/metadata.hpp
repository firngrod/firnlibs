#pragma once
#include <json/value.h>

namespace FirnLibs
{
  namespace Mp3
  {
    bool GetMetadata(Json::Value &metaData, const std::string &fileName, const std::vector<std::string> & needValues, const std::string &defaultval);
  }
}
