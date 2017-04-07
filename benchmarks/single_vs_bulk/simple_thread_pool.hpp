#ifndef SIMPLE_THREAD_POOL_HPP
#define SIMPLE_THREAD_POOL_HPP

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

class simple_thread_pool
{
public:
  explicit simple_thread_pool(std::size_t threads)
  {
    for (std::size_t i = 0; i < threads; ++i)
      threads_.emplace_back([this]{ run(); });
  }

  ~simple_thread_pool()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stopped_ = true;
    condition_.notify_all();
    lock.unlock();
    for (auto& t : threads_)
      t.join();
  }

  void join()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    joined_ = true;
    condition_.notify_all();
    lock.unlock();
    for (auto& t : threads_)
      t.join();
    threads_.clear();
  }

  class executor_type
  {
  public:
    template <class Function>
    void execute(Function f) const
    {
      pool_->enqueue(std::move(f));
    }

  private:
    friend class simple_thread_pool;
    simple_thread_pool* pool_;
    explicit executor_type(simple_thread_pool* pool) : pool_(pool) {}
  };

  executor_type executor()
  {
    return executor_type(this);
  }

private:
  struct func_base
  {
    virtual ~func_base() {}
    virtual void call() = 0;
    std::unique_ptr<func_base> next_;
  };

  template <class Function>
  struct func : func_base
  {
    explicit func(Function f) : function_(std::move(f)) {}
    virtual void call()
    {
      std::unique_ptr<func> fp(this);
      Function f(std::move(function_));
      fp.release();
      f();
    }
    Function function_;
  };

  void run()
  {
    for (std::unique_lock<std::mutex> lock(mutex_);; lock.lock())
    {
      condition_.wait(lock, [this]{ return stopped_ || joined_ || head_; });
      if (stopped_ || (joined_ && !head_)) return;
      func_base* func = head_.release();
      head_ = std::move(func->next_);
      tail_ = head_ ? tail_ : &head_;
      lock.unlock();
      func->call();
    }
  }

  template <class Function>
  void execute(Function f)
  {
    std::unique_ptr<func_base> fp(new func<Function>(std::move(f)));
    std::unique_lock<std::mutex> lock(mutex_);
    *tail_ = std::move(fp);
    tail_ = &(*tail_)->next_;
    condition_.notify_one();
  }

  std::mutex mutex_;
  std::condition_variable condition_;
  std::list<std::thread> threads_;
  std::unique_ptr<func_base> head_;
  std::unique_ptr<func_base>* tail_ = &head_;
  bool stopped_ = false;
  bool joined_ = false;
};

#endif // SIMPLE_THREAD_POOL_HPP
