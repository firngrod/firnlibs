#include "networking.hpp"
#include "unistd.h"
#include <iostream>

namespace FirnLibs {


Networking::Client::Client()
{
  identifier = 0;
  limboState = nullptr;
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

bool Networking::Client::Commence(void (*callback)(DataReceivedState *), void *asyncState)
{
  // Save the identifier.
  identifier = limboState->identifier;

  // Message the socket to the polldancer
  // Notice that we intercept the callback to do buffering and data interc...
  // We do data treatment in this class before signalling the user.
  // For their convenience.  Nothing sinister.
  auto lambda = [this](const std::vector<unsigned char> &message) -> void 
  {
    this->HandleIncData(message);
  };
  limboState->msg = ConnectionAdd;
  limboState->pCallback = (void *) new std::function<void (const std::vector<unsigned char> &)>(lambda);
  Networking::GetInstance().AddSocket(*limboState);
  
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
}



}
