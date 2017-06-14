#ifndef ThreadsH
#define ThreadsH
//-----------------------------------------------------------------------------
#include "Pch.h"
//-----------------------------------------------------------------------------
#ifdef __GNUC__
class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&_mtx, NULL);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&_mtx);
    }
    void Lock()
    {
        pthread_mutex_lock(&_mtx);
    }
    void Unlock()
    {
        pthread_mutex_unlock(&_mtx);
    }
private:
    pthread_mutex_t _mtx;
    //mutable CRITICAL_SECTION _mtx;
};
#else
class Mutex
{
  public:
    Mutex()
    {
      ::InitializeCriticalSection(&_mtx);
    }
    ~Mutex()
    {
      ::DeleteCriticalSection(&_mtx);
    }
    void Lock()
    {
      ::EnterCriticalSection(&_mtx);
    }
    void Unlock()
    {
      ::LeaveCriticalSection(&_mtx);
    }
  private:
    mutable CRITICAL_SECTION _mtx;
};
#endif
//-----------------------------------------------------------------------------
template < class Host, class MutexPolicy = Mutex >
class ObjectLevelLockable
{
  private :
    mutable MutexPolicy _mtx;
  public:
    ObjectLevelLockable() : _mtx() {}
    ObjectLevelLockable(const ObjectLevelLockable&) : _mtx() {}
    ~ObjectLevelLockable() {}

  class Lock;
  friend class Lock;

  class Lock
  {
  public:
    explicit Lock(const ObjectLevelLockable& host) : _host(host)
    {
      _host._mtx.Lock();
    }
    explicit Lock(const ObjectLevelLockable* host) : _host(*host)
    {
      _host._mtx.Lock();
    }
    ~Lock()
    {
      _host._mtx.Unlock();
    }
  private:
    Lock();
    Lock(const Lock&);
    Lock& operator=(const Lock&);
    const ObjectLevelLockable& _host;
  };
};
//-----------------------------------------------------------------------------
template <class Host, class MutexPolicy = Mutex >
class ClassLevelLockable
{
  private :
    class Initializer
    {
    public :
      Initializer() : _init(true), _mtx()
      {
      }
      ~Initializer()
      {
      }
    public :
      bool         _init;
      MutexPolicy  _mtx;
    };

    static Initializer _initializer;

  public:
    class Lock;
    friend class Lock;

    class Lock
    {
      public:
        Lock()
        {
          _initializer._mtx.Lock();
        }
        explicit Lock(const ClassLevelLockable&)
        {
          _initializer._mtx.Lock();
        }
        explicit Lock(const ClassLevelLockable*)
        {
          _initializer._mtx.Lock();
        }
        ~Lock()
        {
          _initializer._mtx.Unlock();
        }
      private:
        Lock(const Lock&);
        Lock& operator=(const Lock&);
    };
};
template < class Host, class MutexPolicy >
typename ClassLevelLockable< Host, MutexPolicy >::Initializer
         ClassLevelLockable< Host, MutexPolicy >::_initializer;
//-----------------------------------------------------------------------------
#endif
