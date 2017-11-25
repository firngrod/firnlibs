#include "networking.hpp"
#include "unistd.h"
#include <iostream>

namespace FirnLibs {


Networking::Client::Client() :
sharedThis(new FirnLibs::Threading::GuardedVar<Client *>(this))
{
  identifier = 0;
  limboState = nullptr;
  errorNo = 0;
}

Networking::Client::~Client()
{
  {
    auto token = sharedThis->Get("Client deconstructor");
    token = nullptr;
  }
  if(limboState != nullptr)
  {
    close(limboState->fd);
    delete (sockaddr *)limboState->pAddr;
    delete limboState;
  }
  else
  {
    Networking::GetInstance().SignalCloseSocket(identifier);
    identifier = 0;
  }
}

bool Networking::Client::Commence(const std::function<void (const std::vector<unsigned char> &)> callback,
                                  const std::function<void (const int &)> errorCallback)
{
  // Sanity check
  //if(callback == nullptr)
    //return false;

  // Save the identifier.
  identifier = limboState->identifier;

  // Save the callbacks.
  this->callback = callback;
  this->errorCallback = errorCallback;

  // Message the socket to the polldancer
  // Notice that we intercept the callback to do buffering and data interc...
  // We do data treatment in this class before signalling the user.
  // For their convenience.  Nothing sinister.
  auto sharedCpy = new std::shared_ptr<FirnLibs::Threading::GuardedVar<Client *> >(sharedThis);
  auto lambda = [sharedCpy](const std::vector<unsigned char> &message) -> void 
  {
    auto token = (*sharedCpy)->Get("Client data callback");
    if((Client *)token != nullptr)
      ((Client *)token)->HandleIncData(message);
  };

  auto errorLambda = [sharedCpy](const int &errorNo) -> void
  {
    auto token = (*sharedCpy)->Get("Client error callback");
    if((Client *)token != nullptr)
    {
      ((Client *)token)->ErrorCallback(errorNo);
    }
  };

  auto cleanupLambda = [sharedCpy]() -> void
  {
    // When this reaches the front of the queue, all callbacks on this class are either done or in progress.
    // If we wait until we can lock, we should be good.
    // This may still be capable of breakage if two threads start "simultaneously" and the other one gets the lock first.
    {
      auto token = (*sharedCpy)->Get("Client cleanup callback");
    }
    delete sharedCpy;
  };
    
  limboState->msg = ConnectionAdd;
  limboState->pCallback = (void *) new std::function<void (const std::vector<unsigned char> &)>(lambda);
  limboState->pErrorCallback = new std::function<void (const int &)>(errorLambda);
  limboState->pCleanupCallback = new std::function<void ()>(cleanupLambda);
  Networking::GetInstance().SignalSocket(*limboState);
  
  // Clean up.
  // NOTICE:  Do not delete the addr.  It will be deleted by the polldancer.
  delete limboState;
  limboState = nullptr;
}

void Networking::Client::HandleIncData(const std::vector<unsigned char> &data)
{
  if(callback != nullptr)
    callback(data);
}

void Networking::Client::ErrorCallback(const int &errorNo)
{
  // If this has been called, something is wrong.
  this->errorNo = errorNo;
  this->identifier = identifier;

  // If they gave us a callback method, relay
  if(errorCallback != nullptr)
    errorCallback(errorNo);
}

void Networking::Client::Send(const std::vector<unsigned char> &data)
{
  auto lock = sharedThis->Get("Client Send");
  PipeMessagePack messagePack;
  messagePack.identifier = identifier;
  messagePack.msg = ConnectionQueueData;
  messagePack.pDataBuf = new std::vector<unsigned char>(data);

  Networking::GetInstance().SignalSocket(messagePack);
}

bool Networking::Client::Connect(const int &port, const std::string &address, const std::string &localAddress,
                                 const std::function<void (const std::vector<unsigned char> &)> &callback,
                                 const std::function<void (const int &)> &errorCallback)
{
  auto lock = sharedThis->Get("Client connect");
  if(identifier != 0)
    return false;

  // Message the socket to the polldancer
  // Notice that we intercept the callback to do buffering and data interc...
  // We do data treatment in this class before signalling the user.
  // For their convenience.  Nothing sinister.
  auto sharedCpy = new std::shared_ptr<FirnLibs::Threading::GuardedVar<Client *> >(sharedThis);
  auto lambda = [sharedCpy](const std::vector<unsigned char> &message) -> void 
  {
    auto token = (*sharedCpy)->Get("Client Connect data callback");
    if((Client *)token != nullptr)
      ((Client *)token)->HandleIncData(message);
  };

  auto errorLambda = [sharedCpy](const int &errorNo) -> void
  {
    auto token = (*sharedCpy)->Get("Client Connect error callback");
    if((Client *)token != nullptr)
    {
      ((Client *)token)->ErrorCallback(errorNo);
    }
  };

  auto cleanupLambda = [sharedCpy]() -> void
  {
    // When this reaches the front of the queue, all callbacks on this class are either done or in progress.
    // If we wait until we can lock, we should be good.
    // This may still be capable of breakage if two threads start "simultaneously" and the other one gets the lock first.
    {
      auto token = (*sharedCpy)->Get("Client Connect cleanup callback");
    }
    delete sharedCpy;
  };
    
  identifier = Networking::GetInstance().ConnectTCP(port, address, localAddress, callback, errorCallback, cleanupLambda);

  if(identifier == 0)
    return false;

  this->callback = callback;
  this->errorCallback = errorCallback;

  return true;
}



}
