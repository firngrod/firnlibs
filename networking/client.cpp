#include "networking.hpp"
#include "unistd.h"
#include <iostream>

namespace FirnLibs {


Networking::Client::Client()
{
  identifier = 0;
  limboState = NULL;
}

Networking::Client::~Client()
{
  if(limboState != NULL)
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
  auto lambda = [](void * state) -> void 
  {
    MessageForwardStruct * pState = (MessageForwardStruct *)state; 
    pState->client->HandleIncData(pState->message);
    delete pState;
  };
  limboState->msg = ConnectionAdd;
  limboState->callback = lambda;
  limboState->state = this;
  Networking::GetInstance().AddSocket(*limboState);
  
  // Clean up.
  // NOTICE:  Do not delete the addr.  It will be deleted by the polldancer.
  delete limboState;
  limboState = NULL;
}

void Networking::Client::HandleIncData(const std::vector<unsigned char> &data)
{
  std::cout << "Received data: " << (char *)&data[0];
}



}
