#pragma once

namespace FirnLibs
{
  namespace System
  {
    // Get the number of logical processors available.
    // It almost feels dumb to have std::thread::hardware_concurrency() wrapped, but this way, I can remember it.
    // Also, this keeps the thread header out of the other files, which is good if you want the processor count but don't do threading, I guess.
    int GetProcessorCount();
  }
}
