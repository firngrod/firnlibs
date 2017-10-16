#pragma once

#ifndef _Networking_h_
#error Included listener.hpp directly!  Include networking.hpp instead.
#endif

    class Listener
    {
    public:
      Listener() { identifier = 0;}
      ~Listener();

      struct AcceptState
      {
        void * asyncState;
        std::shared_ptr<Client> client;
      };
      bool Listen(const int &port, void (*callback)(AcceptState *), void * asyncState);
      void Stop();
      // Prevent copying to avoid issues with deconstruction;
      Listener(const Listener &) = delete;
      void operator=(const Listener &) = delete;
    protected:
      int identifier;
      friend Networking;
    };


