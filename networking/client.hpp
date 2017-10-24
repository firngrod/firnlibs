#pragma once

#ifndef _Networking_h_
#error Included client.hpp directly!  Include networking.hpp instead.
#endif



    class Client
    {
    public:
      Client();
      ~Client();

      class DataReceivedState
      {
        void * asyncState;
        std::vector<unsigned char> message;
        std::vector<unsigned char> reply;
      };
      bool Commence(const std::function<void (const std::vector<unsigned char> &)> callback,
                    const std::function<void (const int &)> errorCallback = nullptr);
      void Send(const std::vector<unsigned char> &data);
    protected:
      int identifier;
      int errorNo;

      // This is used to handle thread safety of the class.  All callbacks from deeper down lock this before going.
      // Anything callable from outside should likewise lock it.
      // The shared_ptr means that we can pass this to callbacks, which then theck the content for validity before running.
      // Upon class instance deconstruction, the content of the guarded pointer is set to nullptr, signifying to callbacks that they need to fail.
      // When the connection is finally destroyed by the polldancer, a cleanup is queued in which a dangling copy of this shared_ptr is deconstructed.
      // Because the queue is fifo and because of the mutex of the variable, this can be used to ensure that all queued functions get to finish.
      std::shared_ptr<FirnLibs::Threading::GuardedVar<Client *> > sharedThis;

      void HandleIncData(const std::vector<unsigned char> &message);
      void ErrorCallback(const int &errorNo);

      // Callbacks from outside;
      std::function<void (const std::vector<unsigned char> &)> callback;
      std::function<void (const int &)> errorCallback;

      PipeMessagePack * limboState;
      friend Networking;
    };
