#ifndef STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H
#define STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H

#include <experimental/bits/rebind_member_result.h>
#include <experimental/bits/always_ready_future.h>
#include <future>
#include <type_traits>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace rebind_impl {

// Default rebind for one way leaves one way executors as is.

template<class Executor>
  constexpr typename std::enable_if<is_one_way_executor<Executor>::value
    && !has_rebind_member<Executor, one_way_t>::value, Executor>::type
      rebind(Executor ex, one_way_t) { return std::move(ex); }

// Default rebind for two way adapts one way executors, leaves two way executors as is.

template <class InnerExecutor>
class two_way_adapter
{
  InnerExecutor inner_ex_;

public:
  two_way_adapter(InnerExecutor ex) : inner_ex_(std::move(ex)) {}

  InnerExecutor rebind(one_way_t) const & { return inner_ex_; }
  InnerExecutor rebind(one_way_t) && { return std::move(inner_ex_); }
  two_way_adapter rebind(two_way_t) const & { return *this; }
  two_way_adapter rebind(two_way_t) && { return std::move(*this); }

  template <class... T> auto rebind(T&&... t) const &
    -> two_way_adapter<typename rebind_member_result<InnerExecutor, T...>::type>
      { return { inner_ex_.rebind(std::forward<T>(t)...) }; }
  template <class... T> auto rebind(T&&... t) &&
    -> two_way_adapter<typename rebind_member_result<InnerExecutor&&, T...>::type>
      { return { std::move(inner_ex_).rebind(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const two_way_adapter& a, const two_way_adapter& b) noexcept
  {
    return a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const two_way_adapter& a, const two_way_adapter& b) noexcept
  {
    return !(a == b);
  }

  template <class Function>
  auto operator()(Function f) const -> std::future<decltype(f())>
  {
    std::packaged_task<decltype(f())()> task(std::move(f));
    std::future<decltype(f())> future = task.get_future();
    inner_ex_(std::move(task));
    return future;
  }
};

template<class Executor>
  typename std::enable_if<is_one_way_executor<Executor>::value && !is_two_way_executor<Executor>::value
    && !has_rebind_member<Executor, two_way_t>::value, two_way_adapter<Executor>>::type
      rebind(Executor ex, two_way_t) { return two_way_adapter<Executor>(std::move(ex)); }

template<class Executor>
  constexpr typename std::enable_if<is_two_way_executor<Executor>::value
    && !has_rebind_member<Executor, two_way_t>::value, Executor>::type
      rebind(Executor ex, two_way_t) { return std::move(ex); }


// Default rebind for always blocking adapts possibly and never blocking executors, leaves always blocking executors as is.

template<class PossiblyBlockingExecutor>
class always_blocking_adapter
{
  using executor_type = decltype(execution::rebind(std::declval<PossiblyBlockingExecutor>(), two_way));

  executor_type two_way_possibly_blocking_ex_;

public:
  always_blocking_adapter(PossiblyBlockingExecutor ex) : two_way_possibly_blocking_ex_(execution::rebind(std::move(ex), two_way)) {}

  PossiblyBlockingExecutor rebind(possibly_blocking_t) const & { return two_way_possibly_blocking_ex_; }
  PossiblyBlockingExecutor rebind(possibly_blocking_t) && { return std::move(two_way_possibly_blocking_ex_); }
  always_blocking_adapter rebind(always_blocking_t) const & { return *this; }
  always_blocking_adapter rebind(always_blocking_t) && { return std::move(*this); }

  template <class... T> auto rebind(T&&... t) const &
    -> always_blocking_adapter<typename rebind_member_result<executor_type, T...>::type>
      { return { two_way_possibly_blocking_ex_.rebind(std::forward<T>(t)...) }; }
  template <class... T> auto rebind(T&&... t) &&
    -> always_blocking_adapter<typename rebind_member_result<executor_type&&, T...>::type>
      { return { std::move(two_way_possibly_blocking_ex_).rebind(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return two_way_possibly_blocking_ex_.context(); }

  friend bool operator==(const always_blocking_adapter& a, const always_blocking_adapter& b) noexcept
  {
    return a.two_way_possibly_blocking_ex_ == b.two_way_possibly_blocking_ex_;
  }

  friend bool operator!=(const always_blocking_adapter& a, const always_blocking_adapter& b) noexcept
  {
    return !(a == b);
  }

  template <class Function>
  auto operator()(Function f) const -> executor_future_t<PossiblyBlockingExecutor,decltype(f())>
  {
    auto future = two_way_possibly_blocking_ex_(std::move(f));
    future.wait();
    return future;
  }
};

template<class Executor>
  constexpr typename std::enable_if<has_rebind_member<Executor, always_blocking_t>::value, Executor>::type
    rebind(Executor ex, always_blocking_t) { return std::move(ex); }

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, always_blocking_t>::value, always_blocking_adapter<Executor>>::type
    rebind(Executor ex, always_blocking_t) { return always_blocking_adapter<Executor>(std::move(ex)); }
    
// Default rebind for possibly blocking does no adaptation, as all executors are possibly blocking.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, possibly_blocking_t>::value, Executor>::type
    rebind(Executor ex, possibly_blocking_t) { return std::move(ex); }

// Default rebind for bulk adapts single executors, leaves bulk executor as is.
template<class SingleExecutor>
class bulk_adapter
{
  using executor_type = decltype(execution::rebind(std::declval<SingleExecutor>(), always_blocking));
  executor_type always_blocking_single_ex_;

public:
  using bulk_forward_progress_guarantee = bulk_unsequenced_execution;

  template<class T>
  using future = impl::always_ready_future<T>;

  bulk_adapter(SingleExecutor ex) : always_blocking_single_ex_(execution::rebind(std::declval<SingleExecutor>(), always_blocking)) {}

  SingleExecutor rebind(single_t) const & { return always_blocking_single_ex_; }
  SingleExecutor rebind(single_t) && { return std::move(always_blocking_single_ex_); }
  bulk_adapter rebind(bulk_t) const & { return *this; }
  bulk_adapter rebind(bulk_t) && { return std::move(*this); }

  template <class... T> auto rebind(T&&... t) const &
    -> bulk_adapter<typename rebind_member_result<executor_type, T...>::type>
      { return { always_blocking_single_ex_.rebind(std::forward<T>(t)...) }; }
  template <class... T> auto rebind(T&&... t) &&
    -> bulk_adapter<typename rebind_member_result<executor_type&&, T...>::type>
      { return { std::move(always_blocking_single_ex_).rebind(std::forward<T>(t)...) }; }

  auto& context() const noexcept { return always_blocking_single_ex_.context(); }

  friend bool operator==(const bulk_adapter& a, const bulk_adapter& b) noexcept
  {
    return a.always_blocking_single_ex_ == b.always_blocking_single_ex_;
  }

  friend bool operator!=(const bulk_adapter& a, const bulk_adapter& b) noexcept
  {
    return !(a == b);
  }

  // two-way operator()
  template <class Function, class ResultFactory, class SharedFactory>
  future<typename std::result_of<ResultFactory()>::type> operator()(Function f, size_t n, ResultFactory rf, SharedFactory sf) const
  {
    using result_type = typename std::result_of<ResultFactory()>::type;

    result_type result = rf();
    auto shared = sf();

    for(size_t i = 0; i < n; ++i)
    {
      always_blocking_single_ex_([=,&result,&shared]
      {
        f(i, result, shared);
      });
    }

    return impl::make_always_ready_future<result_type>(std::move(result));
  }

  // one-way operator()
  template <class Function, class SharedFactory>
  void operator()(Function f, size_t n, SharedFactory sf) const
  {
    auto shared = sf();

    for(size_t i = 0; i < n; ++i)
    {
      always_blocking_single_ex_([=,&shared]
      {
        f(i, shared);
      });
    }
  }
};

// Default rebind for continuation mode does no adaptation.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_continuation_t) { return std::move(ex); }

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_continuation_t>::value, Executor>::type
    rebind(Executor ex, is_not_continuation_t) { return std::move(ex); }

// Default rebind for work mode does no adaptation.

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_work_t>::value, Executor>::type
    rebind(Executor ex, is_work_t) { return std::move(ex); }

template<class Executor>
  constexpr typename std::enable_if<!has_rebind_member<Executor, is_not_work_t>::value, Executor>::type
    rebind(Executor ex, is_not_work_t) { return std::move(ex); }

// Default rebind for allocator does no adaptation.

template<class Executor, class ProtoAllocator>
  constexpr typename std::enable_if<!has_rebind_member<Executor, std::allocator_arg_t, ProtoAllocator>::value, Executor>::type
    rebind(Executor ex, std::allocator_arg_t, const ProtoAllocator&) { return std::move(ex); }

} // namespace rebind_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_REBIND_ADAPTATIONS_H
