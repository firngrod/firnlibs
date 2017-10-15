#include "networking.hpp"
namespace FirnLibs {

bool Networking::Listener::Listen(const int &port, void (*callback)(AcceptState *), void * callbackState)
{
  // If this already has an fd, don't overwrite it with a new 
  if(fd != -1)
    return false;

  // We need a callback.  Not necessarily a state.
  if(callback == NULL)
    return false;

  fd = Networking::GetInstance().Listen(port, callback, callbackState);

  if(fd == -1)
    return false;

  return true;
}


Networking::Listener::~Listener()
{
  Stop();
}

void Networking::Listener::Stop()
{
  Networking::GetInstance().SignalCloseSocket(fd);
  fd = -1;
}

}
