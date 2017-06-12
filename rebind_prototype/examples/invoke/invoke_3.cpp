#include <experimental/thread_pool>
#include <iostream>
#include <tuple>
#include <cassert>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class Executor, class Function, class Args, std::size_t... I>
auto invoke_helper(Executor ex, Function f, Args args, std::index_sequence<I...>)
{
  return execution::rebind(execution::rebind(ex, execution::two_way), execution::always_blocking)(
      [f = std::move(f), args = std::move(args)]() mutable
      {
        return f(std::move(std::get<I>(args))...);
      }).get();
}

template <class Executor, class Function, class... Args>
auto invoke(Executor ex, Function f, Args&&... args)
{
  return invoke_helper(std::move(ex), std::move(f),
      std::make_tuple(std::forward<Args>(args)...),
      std::make_index_sequence<sizeof...(Args)>());
}

class inline_executor
{
public:
  auto& context() const noexcept { return *this; }

  friend bool operator==(const inline_executor&, const inline_executor&) noexcept
  {
    return true;
  }

  friend bool operator!=(const inline_executor&, const inline_executor&) noexcept
  {
    return false;
  }

  template <class Function>
  void operator()(Function f) const noexcept
  {
    f();
  }
};

int main()
{
  int result = invoke(inline_executor(), [](int i, int j){ return i + j; }, 20, 22);
  std::cout << "result is " << result << "\n";

  assert(result == 42);
}
