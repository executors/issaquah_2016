#ifndef STD_EXPERIMENTAL_BITS_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_EXECUTOR_H

#include <atomic>
#include <experimental/bits/bad_executor.h>
#include <memory>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

class executor
{
  struct func_base
  {
    virtual ~func_base() {}
    virtual void call() = 0;
  };

  template<class Function>
  struct func : func_base
  {
    Function function_;

    explicit func(Function f) : function_(std::move(f)) {}

    virtual void call()
    {
      std::unique_ptr<func> fp(this);
      Function f(std::move(function_));
      fp.reset();
      f();
    }
  };

  struct shared_factory_base
  {
    virtual ~shared_factory_base() {}
    virtual std::shared_ptr<void> call() = 0;
  };

  template<class SharedFactory>
  struct shared_factory : shared_factory_base
  {
    SharedFactory factory_;

    explicit shared_factory(SharedFactory sf) : factory_(std::move(sf)) {}

    virtual std::shared_ptr<void> call()
    {
      return std::make_shared<decltype(factory_())>(factory_());
    }
  };

  struct bulk_func_base
  {
    virtual ~bulk_func_base() {}
    virtual void call(std::size_t i, std::shared_ptr<void>& s) = 0;
  };

  template<class Function, class SharedState>
  struct bulk_func : bulk_func_base
  {
    Function function_;

    explicit bulk_func(Function f) : function_(std::move(f)) {}

    virtual void call(std::size_t i, std::shared_ptr<void>& s)
    {
      function_(i, *std::static_pointer_cast<SharedState>(s));
    }
  };

  struct impl_base
  {
    virtual ~impl_base() {}
    virtual impl_base* clone() const noexcept = 0;
    virtual void destroy() noexcept = 0;
    virtual void executor_execute(std::unique_ptr<func_base> f) = 0;
    virtual void executor_bulk_execute(std::unique_ptr<bulk_func_base> f, std::size_t n, std::shared_ptr<shared_factory_base> sf) = 0;
    virtual const type_info& executor_target_type() const = 0;
    virtual void* executor_target() = 0;
    virtual const void* executor_target() const = 0;
    virtual bool executor_equals(const impl_base* e) const noexcept = 0;
    virtual impl_base* executor_rebind(never_blocking_t) const = 0;
    virtual impl_base* executor_rebind(possibly_blocking_t) const = 0;
    virtual impl_base* executor_rebind(always_blocking_t) const = 0;
    virtual impl_base* executor_rebind(is_continuation_t) const = 0;
    virtual impl_base* executor_rebind(is_not_continuation_t) const = 0;
    virtual impl_base* executor_rebind(is_work_t) const = 0;
    virtual impl_base* executor_rebind(is_not_work_t) const = 0;
    virtual const type_info& context_target_type() const = 0;
    virtual const void* context_target() const = 0;
    virtual bool context_equals(const impl_base* e) const noexcept = 0;
  };

  template<class Executor>
  struct impl : impl_base
  {
    Executor executor_;
    std::atomic<std::size_t> ref_count_{1};

    explicit impl(Executor ex) : executor_(std::move(ex)) {}

    virtual impl_base* clone() const noexcept
    {
      impl* e = const_cast<impl*>(this);
      ++e->ref_count_;
      return e;
    }

    virtual void destroy() noexcept
    {
      if (--ref_count_ == 0)
        delete this;
    }

    virtual void executor_execute(std::unique_ptr<func_base> f)
    {
      executor_.execute([f = std::move(f)]() mutable { f.release()->call(); });
    }

    virtual void executor_bulk_execute(std::unique_ptr<bulk_func_base> f, std::size_t n, std::shared_ptr<shared_factory_base> sf)
    {
      executor_.bulk_execute(
          [f = std::move(f)](std::size_t i, auto s) mutable { f->call(i, s); }, n,
          [sf = std::move(sf)]() mutable { return sf->call(); });
    }

    virtual const type_info& executor_target_type() const
    {
      return typeid(executor_);
    }

    virtual void* executor_target()
    {
      return &executor_;
    }

    virtual const void* executor_target() const
    {
      return &executor_;
    }

    virtual bool executor_equals(const impl_base* e) const noexcept
    {
      if (this == e)
        return true;
      if (executor_target_type() != e->executor_target_type())
        return false;
      return executor_ == *static_cast<const Executor*>(e->executor_target());
    }

    virtual impl_base* executor_rebind(never_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, never_blocking))>(execution::rebind(executor_, never_blocking));
    }

    virtual impl_base* executor_rebind(possibly_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, possibly_blocking))>(execution::rebind(executor_, possibly_blocking));
    }

    virtual impl_base* executor_rebind(always_blocking_t) const
    {
      return new impl<decltype(execution::rebind(executor_, always_blocking))>(execution::rebind(executor_, always_blocking));
    }

    virtual impl_base* executor_rebind(is_continuation_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_continuation))>(execution::rebind(executor_, is_continuation));
    }

    virtual impl_base* executor_rebind(is_not_continuation_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_not_continuation))>(execution::rebind(executor_, is_not_continuation));
    }

    virtual impl_base* executor_rebind(is_work_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_work))>(execution::rebind(executor_, is_work));
    }

    virtual impl_base* executor_rebind(is_not_work_t) const
    {
      return new impl<decltype(execution::rebind(executor_, is_not_work))>(execution::rebind(executor_, is_not_work));
    }

    virtual const type_info& context_target_type() const
    {
      return typeid(executor_.context());
    }

    virtual const void* context_target() const
    {
      return &executor_.context();
    }

    virtual bool context_equals(const impl_base* i) const noexcept
    {
      if (this == i)
        return true;
      if (context_target_type() != i->context_target_type())
        return false;
      return executor_.context() == *static_cast<const executor_context_t<Executor>*>(i->context_target());
    }
  };

public:
  class context_type
  {
    friend class executor;
    impl_base *impl_{nullptr};
    context_type() {};

  public:
    context_type(const context_type&) = delete;
    context_type& operator=(const context_type&) = delete;

    friend bool operator==(const context_type& a, const context_type& b) noexcept
    {
      if (!a.impl_ && !b.impl_)
        return true;
      if (a.impl_ && b.impl_)
        return a.impl_->context_equals(b.impl_);
      return false;
    }

    friend bool operator!=(const context_type& a, const context_type& b) noexcept
    {
      return !(a == b);
    }

    template<class Context>
    friend bool operator==(const context_type& a, const Context& b) noexcept
    {
      if (a.impl_ && a.impl_->context_target_type() == typeid(Context))
        return *static_cast<const Context*>(a.impl_->context_target()) == b;
      return false;
    }

    template<class Context>
    friend bool operator!=(const context_type& a, const Context& b) noexcept
    {
      return !(a == b);
    }

    template<class Context>
    friend bool operator==(const Context& a, const context_type& b) noexcept
    {
      return b == a;
    }

    template<class Context>
    friend bool operator!=(const Context& a, const context_type& b) noexcept
    {
      return !(b == a);
    }
  };

  // construct / copy / destroy:

  executor() noexcept {}
  executor(std::nullptr_t) noexcept {}

  executor(const executor& e) noexcept
  {
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
  }

  executor(executor&& e) noexcept
  {
    context_.impl_ = e.context_.impl_;
    e.context_.impl_ = nullptr;
  }

  template<class Executor> executor(Executor e)
  {
    auto e2 = execution::rebind(execution::rebind(std::move(e), execution::bulk), execution::two_way);
    context_.impl_ = new impl<decltype(e2)>(std::move(e2));
  }

  executor& operator=(const executor& e) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = e.context_.impl_ ? e.context_.impl_->clone() : nullptr;
    return *this;
  }

  executor& operator=(executor&& e) noexcept
  {
    if (this != &e)
    {
      if (context_.impl_) context_.impl_->destroy();
      context_.impl_ = e.context_.impl_;
      e.context_.impl_ = nullptr;
    }
    return *this;
  }

  executor& operator=(nullptr_t) noexcept
  {
    if (context_.impl_) context_.impl_->destroy();
    context_.impl_ = nullptr;
    return *this;
  }

  template<class Executor> executor& operator=(Executor e)
  {
    return operator=(executor(std::move(e)));
  }

  ~executor()
  {
    if (context_.impl_) context_.impl_->destroy();
  }

  // polymorphic executor modifiers:

  void swap(executor& other) noexcept
  {
    std::swap(context_.impl_, other.context_.impl_);
  }

  template<class Executor> void assign(Executor e)
  {
    operator=(executor(std::move(e)));
  }

  // executor operations:

  executor rebind(never_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(never_blocking) : context_.impl_->clone(); }
  executor rebind(possibly_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(possibly_blocking) : context_.impl_->clone(); }
  executor rebind(always_blocking_t) const { return context_.impl_ ? context_.impl_->executor_rebind(always_blocking) : context_.impl_->clone(); }
  executor rebind(is_continuation_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_continuation) : context_.impl_->clone(); }
  executor rebind(is_not_continuation_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_not_continuation) : context_.impl_->clone(); }
  executor rebind(is_work_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_work) : context_.impl_->clone(); }
  executor rebind(is_not_work_t) const { return context_.impl_ ? context_.impl_->executor_rebind(is_not_work) : context_.impl_->clone(); }
  
  const context_type& context() const noexcept
  {
    return context_;
  }

  template<class Function> void execute(Function f) const
  {
    std::unique_ptr<func_base> fp(new func<Function>(std::move(f)));
    context_.impl_ ? context_.impl_->executor_execute(std::move(fp)) : throw bad_executor();
  }

#if 0 // TODO implement single two-way support.
  template<class Function> auto async_execute(Function f) const -> std::future<decltype(f())>;
#endif

  template<class Function, class SharedFactory> void bulk_execute(Function f, std::size_t n, SharedFactory sf) const
  {
    std::unique_ptr<bulk_func_base> fp(new bulk_func<Function, decltype(sf())>(std::move(f)));
    std::shared_ptr<shared_factory_base> sfp(new shared_factory<SharedFactory>(std::move(sf)));
    context_.impl_ ? context_.impl_->executor_bulk_execute(std::move(fp), n, std::move(sfp)) : throw bad_executor();
  }

#if 0 // TODO implement bulk two-way support.
  template<class Function, class ResultFactory, class SharedFactory>
    auto bulk_async_execute(Function f, std::size_t n, ResultFactory rf, SharedFactory sf) const -> std::future<decltype(rf())>;
#endif

  // polymorphic executor capacity:

  explicit operator bool() const noexcept
  {
    return !!context_.impl_;
  }

  // polymorphic executor target access:

  const type_info& target_type() const noexcept
  {
    return context_.impl_ ? context_.impl_->executor_target_type() : typeid(void);
  }

  template<class Executor> Executor* target() noexcept
  {
    return context_.impl_ ? static_cast<Executor*>(context_.impl_->executor_target()) : nullptr;
  }

  template<class Executor> const Executor* target() const noexcept
  {
    return context_.impl_ ? static_cast<Executor*>(context_.impl_->executor_target()) : nullptr;
  }

  // polymorphic executor comparisons:

  friend bool operator==(const executor& a, const executor& b) noexcept
  {
    if (!a.get_impl() && !b.get_impl())
      return true;
    if (a.get_impl() && b.get_impl())
      return a.get_impl()->executor_equals(b.get_impl());
    return false;
  }

  friend bool operator==(const executor& e, nullptr_t) noexcept
  {
    return !e;
  }

  friend bool operator==(nullptr_t, const executor& e) noexcept
  {
    return !e;
  }

  friend bool operator!=(const executor& a, const executor& b) noexcept
  {
    return !(a == b);
  }

  friend bool operator!=(const executor& e, nullptr_t) noexcept
  {
    return !!e;
  }

  friend bool operator!=(nullptr_t, const executor& e) noexcept
  {
    return !!e;
  }

private:
  executor(impl_base* i) noexcept { context_.impl_ = i; }
  context_type context_;
  const impl_base* get_impl() const { return context_.impl_; }
};

// executor specialized algorithms:

inline void swap(executor& a, executor& b) noexcept
{
  a.swap(b);
}

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_EXECUTOR_H