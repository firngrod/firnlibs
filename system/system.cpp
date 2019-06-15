#include "system.hpp"
#include <thread>

namespace FirnLibs{ namespace System{


int GetProcessorCount()
{
  return std::thread::hardware_concurrency();
}


}}
