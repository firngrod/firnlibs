#include "threadpool.hpp"
#include <vector>

namespace FirnLibs { namespace Threading {

Threadpool::Threadpool(const size_t &count)
{
  auto tok = finishing.Get();
  tok = false;
  SetThreadCount(count);
}

Threadpool::~Threadpool()
{
  auto tok = finishing.Get();
  tok = true;
  // Remove the threads.  Note that this is blocking.
  SetThreadCount(0);
}

void Threadpool::SetThreadCount(const size_t &count)
{
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

void Threadpool::RunnerFunc(void * params)
{
  // Each thread loops while not told to stop, waiting on a condition variable which will tell it when to run.
  Threadpool::ThreadParams *tParams = (ThreadParams *)params;
  bool isStopTime = false;
  while (!isStopTime)
  {
    FunctionParams funcParams;
    // Wait until a message is received.
    // Function block this to not keep the mutex unnecessarily
    {
      std::unique_lock<std::mutex> lk(*tParams->mutex);
      tParams->cv->wait(lk, [&tParams]{return *tParams->stopSignal || tParams->queue->CanPop();});
      // Pop within the mutex, 'cause why not.
      funcParams = tParams->queue->Pop();
    }
    // Sanity check.  It may have been a fluke, or it may have been a stop signal.
    if(funcParams.Function != NULL)
    {
      funcParams.Function(funcParams.paramStructPtr);
    }
    // Notify in case other thread functions are still waiting.
    tParams->cv->notify_one();
    // Check if it's time to stop.
    isStopTime = *tParams->stopSignal;
  }
  delete tParams->stopSignal;
  delete tParams;
}

bool Threadpool::Push(void (*Function)(void *), void * params)
{
  // Are we trying to wind it down?
  if(finishing.Get())
    return false;

  // Generate a new function package to put on the queue
  FunctionParams funcParams;
  funcParams.Function = Function;
  funcParams.paramStructPtr = params;
  // Push it on the queue.
  queue.Push(funcParams);
  // Notify some thread that there is stuff to do.
  cv.notify_one();
  return true;
}

void Threadpool::FinishUp()
{
  // Signal that we want to wind it down.
  finishing.Get() = true;

  if(!queue.CanPop())
    return;

  std::unique_lock<std::mutex> lk(mutex);
  cv.wait(lk, [this]{return !queue.CanPop();});
}

}}
