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
      class Token
      {
      friend GuardedVar;
      public:
        T *operator->() { return &child; }
        const T *operator->() const { return &child; }
		Token &operator=(const T &other) { child = other; return *this; }
        operator T&() { return child; };
        operator T() const { return child; }
        T Copy() const { return child; }
        T &Copy(T& out) const { return out = child; }
      private:
        Token(T &input, std::recursive_mutex &mute, const std::string &getPoint) : child(input), mut(mute)
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
        T &child;
        std::recursive_mutex &mut;
        std::unique_lock<std::recursive_mutex> lock;
        std::string getPoint;
      };

      GuardedVar() {};
      GuardedVar(const T &input) { child = input; }
      Token Get(const std::string &getPoint) { return Token(child, mut, getPoint); }
      GuardedVar<T> operator=(const GuardedVar<T> &other) = delete;
      GuardedVar<T> (const GuardedVar<T> &other) = delete;

    private:
      T child;
      std::recursive_mutex mut;
    };
  }
}
