#include <experimental/thread_pool>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <cassert>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

template <class Executor, class Function>
typename std::result_of<Function()>::type invoke(Executor ex, Function&& f)
{
  return execution::rebind(execution::rebind(ex, execution::two_way), execution::always_blocking)(std::forward<Function>(f)).get();
}

int main()
{
  static_thread_pool pool{1};
  int result = invoke(pool.executor(), []{ return 42; });
  std::cout << "result is " << result << "\n";

  assert(result == 42);
}
