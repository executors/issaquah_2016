#include <iostream>
#include <experimental/thread_pool>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{4};
  auto ex = pool.executor().rebind(execution::bulk).rebind(execution::one_way).rebind(execution::never_blocking);
  ex([](int n, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 0; });
  pool.wait();
}