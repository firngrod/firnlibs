#pragma once

#ifndef _Networking_h_
#error Included listener.hpp directly!  Include networking.hpp instead.
#endif

    class Listener
    {
    public:
      Listener();
      ~Listener();

      bool Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback,
                  const std::function<void (const int &)> &errorCallback = nullptr);
      void Stop();
      // Prevent copying to avoid issues with deconstruction;
      Listener(const Listener &) = delete;
      void operator=(const Listener &) = delete;
    protected:

      // This is used to handle thread safety of the class.  All callbacks from deeper down lock this before going.
      // Anything callable from outside should likewise lock it.
      // The shared_ptr means that we can pass this to callbacks, which then theck the content for validity before running.
      // Upon class instance deconstruction, the content of the guarded pointer is set to nullptr, signifying to callbacks that they need to fail.
      // When the connection is finally destroyed by the polldancer, a cleanup is queued in which a dangling copy of this shared_ptr is deconstructed.
      // Because the queue is fifo and because of the mutex of the variable, this can be used to ensure that all queued functions get to finish.
      std::shared_ptr<FirnLibs::Threading::GuardedVar<Listener *> > sharedThis;

      // Our own callback functions.
      void ErrorCallback(const int &errorNo);
      void Callback(const std::shared_ptr<Client> &client);

      // Callback functions from outside.
      std::function<void (const std::shared_ptr<Client> &client)> callback;
      std::function<void (const int &)> errorCallback;

      int errorNo;
      unsigned int identifier;
      friend Networking;
    };


