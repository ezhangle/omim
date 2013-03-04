#pragma once

#include "../std/target_os.hpp"

#include "../std/stdint.hpp"
#include "../std/vector.hpp"
#include "../std/utility.hpp"

#ifdef OMIM_OS_WINDOWS
#include "../std/windows.hpp" // for DWORD
#endif

namespace threads
{
  class IRoutine
  {
  private:
    bool m_isCancelled;

  protected:
    inline bool IsCancelled() const { return m_isCancelled; }

  public:
    IRoutine() : m_isCancelled(false) {}
    virtual ~IRoutine() {}

    /// Performing the main task
    virtual void Do() = 0;

    /// Implement this function to respond to the cancellation event.
    /// Cancellation means that IRoutine should exit as fast as possible.
    virtual void Cancel() { m_isCancelled = true; }
  };

  class ThreadImpl;
  /// wrapper for Create and Terminate threads API
  class Thread
  {
    ThreadImpl * m_impl;
    IRoutine * m_routine;

    Thread(Thread const &);
    Thread & operator=(Thread const &);

  public:
    Thread();
    ~Thread();

    /// Run thread immediately
    /// @param pRoutine is owned by Thread class
    bool Create(IRoutine * pRoutine);
    /// Calling the IRoutine::Cancel method, and Join'ing with the task execution.
    void Cancel();
    /// wait for thread ending
    void Join();
  };

  /// Simple threads container. Takes ownership for every added IRoutine.
  class ThreadPool
  {
    typedef pair<Thread *, IRoutine *> ValueT;
    vector<ValueT> m_pool;

    ThreadPool(ThreadPool const &);
    ThreadPool & operator=(Thread const &);

  public:
    ThreadPool(size_t reserve = 0);
    ~ThreadPool();

    void Add(IRoutine * pRoutine);
    void Join();

    IRoutine * GetRoutine(size_t i) const;
  };

  /// Suspends the execution of the current thread until the time-out interval elapses.
  /// @param[in] ms time-out interval in milliseconds
  void Sleep(size_t ms);

#ifdef OMIM_OS_WINDOWS
  typedef DWORD ThreadID;
#else
  typedef void * ThreadID;
#endif

  ThreadID GetCurrentThreadID();

} // namespace threads
