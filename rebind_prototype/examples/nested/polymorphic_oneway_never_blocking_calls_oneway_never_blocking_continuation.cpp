#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

using executor = execution::executor<
  execution::oneway_t,
  execution::single_t,
  execution::always_blocking_t,
  execution::never_blocking_t,
  execution::prefer_only<execution::continuation_t>>;

int main()
{
  static_thread_pool pool{1};
  executor ex = execution::require(pool.executor(), execution::always_blocking);
  std::cout << "before submission\n";
  ex.execute([ex = execution::prefer(execution::require(ex, execution::never_blocking), execution::continuation)]{
      std::cout << "outer starts\n";
      ex.execute([]{ std::cout << "inner\n"; });
      std::cout << "outer ends\n";
    });
  std::cout << "after submission, before wait\n";
  pool.wait();
  std::cout << "after wait\n";
}
