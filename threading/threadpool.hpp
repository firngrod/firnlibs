#pragma once
#include "queue.hpp"
#include "guardedvar.hpp"
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
      // Initialize the threadpool with a number of slave threads.
      Threadpool(const size_t &count);
      
      // Beware when deconstructing the threadpool since this will leak all unfinished queued jobs.
      // It is recommended to do a FinishUp() first, although if terminating the program, you may not care.
      ~Threadpool();

      // Update the amount of available threads.
      void SetThreadCount(const size_t &count);

      // Push another job to the threadpool.
      // Returns false if the threadpool is trying to wind down.
      // Check for this case and be sure to clean up since the child thread won't run.
      bool Push(void (*Function)(void *), void * params);

      // Prevents further jobs from being added to the queue and blocks until all jobs have finished.
      void FinishUp();

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
      std::mutex mutex, threadMutex;

      static void RunnerFunc(void * params);
      FirnLibs::Threading::GuardedVar<bool> finishing;
    };
  }
}
