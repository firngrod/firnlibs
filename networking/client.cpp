#include "networking.hpp"
#include "unistd.h"
#include <iostream>

namespace FirnLibs {


Networking::Client::Client()
{
  identifier = 0;
  limboState = nullptr;
  errorNo = 0;
}

Networking::Client::~Client()
{
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
  auto lambda = [this](const std::vector<unsigned char> &message) -> void 
  {
    this->HandleIncData(message);
  };
  auto errorLambda = [this](const int &errorNo) -> void
  {
    this->ErrorCallback(errorNo);
  };
  limboState->msg = ConnectionAdd;
  limboState->pCallback = (void *) new std::function<void (const std::vector<unsigned char> &)>(lambda);
  limboState->pErrorCallback = new std::function<void (const int &)>(errorLambda);
  Networking::GetInstance().SignalSocket(*limboState);
  
  // Clean up.
  // NOTICE:  Do not delete the addr.  It will be deleted by the polldancer.
  delete limboState;
  limboState = nullptr;
}

void Networking::Client::HandleIncData(const std::vector<unsigned char> &data)
{
  std::vector<unsigned char> data2 = data;
  data2.push_back('\0');
  std::cout << "Received data.  Size: " << data.size() << "  Content: " <<  (char *)&data2[0];
  std::cout << "Echoing the data back to sender.\n";
  Send(data);
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
  PipeMessagePack messagePack;
  messagePack.identifier = identifier;
  messagePack.msg = ConnectionQueueData;
  messagePack.pDataBuf = new std::vector<unsigned char>(data);

  Networking::GetInstance().SignalSocket(messagePack);
}


}
