#include "general.hpp"
#include <cstring>

namespace FirnLibs { namespace Crypto {

void StringToVector(ByteVector &theVector, const std::string &theString)
{
  theVector.resize(theString.size(), 0);
  memcpy(&theVector[0], theString.c_str(), theVector.size());
}

void VectorToString(std::string &theString, const ByteVector &theVector)
{
  theString = (const char *)&theVector[0];
}

}}
