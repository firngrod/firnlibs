#include "threadpool.hpp"
#include "../system/system.hpp"
#include <vector>

namespace FirnLibs { namespace Threading {

Threadpool::Threadpool(const size_t &threadCount)
{
  auto tok = finishing.Get("Threadpool constructor");
  tok = false;
  SetThreadCount(threadCount);
}

Threadpool::~Threadpool()
{
  auto tok = finishing.Get("Threadpool deconstructor");
  tok = true;
  // Remove the threads.  Note that this is blocking.
  SetThreadCount(0);
}

void Threadpool::SetThreadCount(const size_t &threadCount)
{
  size_t count = threadCount;
  if(count == -1)
    count = FirnLibs::System::GetProcessorCount();

  std::lock_guard<std::mutex> threadLock(threadMutex);
  // Ramp up the threads.
  while(threadList.size() < count)
  {
    // Everything the thread needs;
    ThreadParams *threadParams = new ThreadParams();
    threadParams->stopSignal = new bool(false); // This belongs to the thread now.
    threadParams->queue = &queue;
    threadParams->cv = &cv;
    threadParams->mutex = &mutex;

    // Everything we need to remember about the thread;
    ThreadPackage threadPackage;
    threadPackage.stopSignal = threadParams->stopSignal;
    threadPackage.thread = new std::thread(RunnerFunc, threadParams);
    threadList.push_back(threadPackage);
  }
  // Ramp down the threads.
  std::vector<ThreadPackage> toClose;
  while(threadList.size() > count)
  {
    toClose.push_back(threadList.front());
    *threadList.front().stopSignal = true;
    threadList.erase(threadList.begin());
  }
  if(toClose.size() == 0)
    return;

  for(auto pkItr: toClose)
    cv.notify_one();

  for(auto pkItr: toClose)
  {
    pkItr.thread->join();
    delete pkItr.thread;
  }
}

void Threadpool::RunnerFunc(Threadpool::ThreadParams * params)
{
  // Each thread loops while not told to stop, waiting on a condition variable which will tell it when to run.
  bool isStopTime = false;
  while (!isStopTime)
  {
    std::function<void()> func;
    // Wait until a message is received.
    // Function block this to not keep the mutex unnecessarily
    {
      std::unique_lock<std::mutex> lk(*params->mutex);
      params->cv->wait(lk, [&params]{return *params->stopSignal || params->queue->CanPop();});
      // Pop within the mutex, 'cause why not.
      func = params->queue->Pop();
    }
    // Sanity check.  It may have been a fluke, or it may have been a stop signal.
    if(func != nullptr)
      func();
    // Notify in case other thread functions are still waiting.
    params->cv->notify_one();
    // Check if it's time to stop.
    isStopTime = *params->stopSignal;
  }
  delete params->stopSignal;
  delete params;
}

bool Threadpool::Push(std::function<void()> func)
{
  // Are we trying to wind it down?
  if(finishing.Get("Threadpool Push"))
    return false;

  // Push it on the queue.
  queue.Push(func);
  // Notify some thread that there is stuff to do.
  cv.notify_one();
  return true;
}

void Threadpool::FinishUp()
{
  // Signal that we want to wind it down.
  finishing.Get("Threadpool FinishUp") = true;

  if(!queue.CanPop())
    return;

  std::unique_lock<std::mutex> lk(mutex);
  cv.wait(lk, [this]{return !queue.CanPop();});
  SetThreadCount(0);
}

}}
