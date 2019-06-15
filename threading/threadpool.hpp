#pragma once
#include "queue.hpp"
#include "guardedvar.hpp"
#include <thread>
#include <map>
#include <condition_variable>
#include <functional>

namespace FirnLibs
{
  namespace Threading
  {
    class Threadpool
    {
    public:
      // Initialize the threadpool with a number of slave threads.
      // count of -1 (yes, I know, unsigned size, just go with it) sets thread count to processor count
      Threadpool(const size_t &threadCount = -1);
      
      // Beware when deconstructing the threadpool since this will leak all unfinished queued jobs.
      // It is recommended to do a FinishUp() first, although if terminating the program, you may not care.
      ~Threadpool();

      // Update the amount of available threads.
      // count of -1 (yes, I know, unsigned size, just go with it) sets thread count to processor count
      void SetThreadCount(const size_t &threadCount = -1);

      // Push another job to the threadpool.
      // Returns false if the threadpool is trying to wind down.
      // Check for this case and be sure to clean up since the child thread won't run.
      bool Push(std::function<void()> func);

      // Prevents further jobs from being added to the queue and blocks until all jobs have finished.
      void FinishUp();

    protected:
      class ThreadParams
      {
      public:
        bool *stopSignal;
        FirnLibs::Threading::Queue<std::function<void()> > *queue;
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
      FirnLibs::Threading::Queue<std::function<void()> > queue;

      std::condition_variable cv;
      std::mutex mutex, threadMutex;

      static void RunnerFunc(ThreadParams * params);
      FirnLibs::Threading::GuardedVar<bool> finishing;
    };
  }
}
