#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;
using std::experimental::concurrency_v2::future;

int main()
{
  static_thread_pool pool{8};
  auto ex = pool.executor().transform(execution::possibly_blocking);
  future<int> f = ex.bulk_twoway_execute([](int n, int&, int&){ std::cout << "part " << n << "\n"; }, 8, []{ return 42; }, []{ return 0; });
  f.wait();
  std::cout << "result is " << f.get() << "\n";
}
