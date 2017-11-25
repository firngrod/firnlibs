#include "networking.hpp"
#include <iostream>

namespace FirnLibs {

Networking::Listener::Listener() :
sharedThis(new FirnLibs::Threading::GuardedVar<Listener *>(this))
{
  identifier = 0;
  errorNo = 0;
}


Networking::Listener::~Listener()
{
  {
    auto token = sharedThis->Get("Listener deconstructor");
    token = nullptr;
  }
  Stop();
}


bool Networking::Listener::Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback,
                                  const std::function<void (const int &)> &errorCallback)
{
  // If this already has an fd, don't overwrite it with a new 
  if(identifier != 0)
    return false;

  // We need a callback.  Not necessarily a state.
  if(callback == nullptr)
    return false;

  this->callback = callback;
  this->errorCallback = errorCallback;
   
  auto sharedCpy = new std::shared_ptr<FirnLibs::Threading::GuardedVar<Listener *> >(sharedThis);

  auto lambda = [sharedCpy](const std::shared_ptr<Client> &client) -> void
  {
    auto token = (*sharedCpy)->Get("Listener client callback");
    if((Listener *)token != nullptr)
      ((Listener *)token)->Callback(client);
  };
  auto errorLambda = [sharedCpy](const int &errorNo) -> void
  {
    auto token = (*sharedCpy)->Get("Listener error callback");
    if((Listener *)token != nullptr)
      ((Listener *)token)->ErrorCallback(errorNo);
  };
  auto cleanupLambda = [sharedCpy]() -> void
  {
    // When this reaches the front of the queue, all callbacks on this class are either done or in progress.
    // If we wait until we can lock, we should be good.
    // This may still be capable of breakage if two threads start "simultaneously" and the other one gets the lock first.
    {
      auto token = (*sharedCpy)->Get("Listener cleanup callback");
    }
    delete sharedCpy;
  };

  identifier = Networking::GetInstance().Listen(port, lambda, errorLambda, cleanupLambda);

  if(identifier == 0)
    return false;

  return true;
}


void Networking::Listener::Stop()
{
  auto token = sharedThis->Get("Listener Stop");
  Networking::GetInstance().SignalCloseSocket(identifier);
  identifier = 0;
}


void Networking::Listener::Callback(const std::shared_ptr<Client> &client)
{
  callback(client);
}


void Networking::Listener::ErrorCallback(const int &errorNo)
{
  // If this has been called, something went wrong.
  this->errorNo = errorNo;
  identifier = 0;
  
  // If they gave us an error callback, relay.
  if(errorCallback != nullptr)
    errorCallback(errorNo);
}

}
