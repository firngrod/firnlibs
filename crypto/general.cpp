#include "general.hpp"
#include <cstring>

namespace FirnLibs { namespace Crypto {

void StringToVector(ByteVector &theVector, const std::string &theString)
{
  theVector.resize(theString.size(), 0);
  memcpy(&theVector[0], theString.c_str(), theVector.size());
}

ByteVector StringToVector(const std::string &theString)
{
  ByteVector tmp;
  StringToVector(tmp, theString);
  return tmp;
}

void VectorToString(std::string &theString, const ByteVector &theVector)
{
  theString.insert(0, (const char *)&theVector[0], theVector.size());
}

}}
