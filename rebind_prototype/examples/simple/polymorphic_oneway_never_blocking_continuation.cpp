#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

using executor = execution::executor<
  execution::oneway_t,
  execution::single_t,
  execution::never_blocking_t,
  execution::continuation_t>;

int main()
{
  static_thread_pool pool{1};
  executor ex = execution::require(pool.executor(), execution::never_blocking, execution::continuation);
  ex.execute([]{ std::cout << "we made it\n"; });
  pool.wait();
}
