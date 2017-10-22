#include "networking.hpp"
#include <iostream>

namespace FirnLibs {

Networking::Listener::Listener()
{
  identifier = 0;
  errorNo = 0;
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
   
  auto lambda = [this](const std::shared_ptr<Client> &client) -> void
  {
    this->Callback(client);
  };
  auto errorLambda = [this](const int &errorNo) -> void
  {
    this->ErrorCallback(errorNo);
  };
  identifier = Networking::GetInstance().Listen(port, lambda, errorLambda);

  if(identifier == 0)
    return false;

  return true;
}


Networking::Listener::~Listener()
{
  Stop();
}

void Networking::Listener::Stop()
{
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
