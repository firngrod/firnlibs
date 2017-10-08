#pragma once
#include <string>
#include "types.hpp"

namespace FirnLibs
{
  namespace Crypto
  {
    void StringToVector(ByteVector &theVector, const std::string &theString);

    void VectorToString(std::string &theString, const ByteVector &TheVector);
  }
}
