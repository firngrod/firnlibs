#pragma once
#include <list>
#include <mutex>

namespace FirnLibs
{
  namespace Threading
  {
    template<class T> class Queue
    {
    public:
      void Push(const T &element)
      {
        std::lock_guard<std::mutex> theLock(theMutex);
        theQueue.push_back(element);
      }
      T Pop()
      {
        std::lock_guard<std::mutex> theLock(theMutex);
        if(theQueue.size() == 0)
          return T();
        T tempie = theQueue.front();
        theQueue.erase(theQueue.begin());
        return tempie;
      }

      bool CanPop()
      {
        std::lock_guard<std::mutex> theLock(theMutex);
        return theQueue.size() > 0;
      }

      size_t Size()
      {
        std::lock_guard<std::mutex> theLock(theMutex);
        return theQueue.size();
      }

    protected:
      std::list<T> theQueue;
      std::mutex theMutex;
    };
  }
}
      
