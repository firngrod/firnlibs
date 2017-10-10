#pragma once
#include "queue.hpp"
#include <thread>
#include <map>
#include <condition_variable>

namespace FirnLibs
{
  namespace Threading
  {
    class Threadpool
    {
    public:
      Threadpool(const size_t &count);
      ~Threadpool();

      void SetThreadCount(const size_t &count);
      void Push(void (*Function)(void *), void * params);

    protected:
      class FunctionParams
      {
      public:
        void * paramStructPtr;
        void (*Function)(void *);
      };
      class ThreadParams
      {
      public:
        bool *stopSignal;
        FirnLibs::Threading::Queue<FunctionParams> *queue;
        std::condition_variable *cv;
        std::mutex *mutex;
      };
      class ThreadPackage
      {
      public:
        std::thread * thread;
        bool * stopSignal;
      };

      std::list<ThreadPackage> threadList;
      FirnLibs::Threading::Queue<FunctionParams> queue;

      std::condition_variable cv;
      std::mutex mutex;

      static void RunnerFunc(void * params);
    };
  }
}
