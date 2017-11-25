#pragma once
#include <mutex>
#ifdef GUARDEDVARDEBUG
#include <iostream>
#endif

namespace FirnLibs
{
  namespace Threading
  {
    template<class T> class GuardedVar
    {
    public:
      template<class TT> class Token
      {
      friend GuardedVar;
      public:
        TT *operator->() { return &child; }
        Token<TT> &operator=(const TT &other) { child = other; }
        operator TT&() { return child; };
        operator TT() const { return child; }
        TT Copy() const { return child; }
        TT &Copy(TT& out) const { return out = child; }
      private:
        Token(TT &input, std::recursive_mutex &mute, const std::string &getPoint) : child(input), mut(mute)
        {
#ifdef GUARDEDVARDEBUG
          this->getPoint = getPoint;
          if(getPoint != "")
          {
            std::cout << "Locking " << getPoint << std::endl;
          }
#endif
          lock = std::unique_lock<std::recursive_mutex>(mut);
#ifdef GUARDEDVARDEBUG
          if(getPoint != "")
          {
            std::cout << "Locked  " << getPoint << std::endl;
          }
#endif
        };
        TT &child;
        std::recursive_mutex &mut;
        std::unique_lock<std::recursive_mutex> lock;
        std::string getPoint;
      };

      GuardedVar() {};
      GuardedVar(const T &input) { child = input; }
      Token<T> Get(const std::string &getPoint) { return Token<T>(child, mut, getPoint); }
      GuardedVar<T> operator=(const GuardedVar<T> &other) = delete;
      GuardedVar<T> (const GuardedVar<T> &other) = delete;

    private:
      T child;
      std::recursive_mutex mut;
    };
  }
}
