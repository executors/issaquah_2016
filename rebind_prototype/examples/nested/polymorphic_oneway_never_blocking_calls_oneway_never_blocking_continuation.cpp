#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

int main()
{
  static_thread_pool pool{1};
  execution::executor ex = pool.executor().transform_executor(execution::always_blocking);
  std::cout << "before submission\n";
  ex.execute([ex = execution::try_transform_executor(ex.transform_executor(execution::never_blocking), execution::continuation)]{
      std::cout << "outer starts\n";
      ex.execute([]{ std::cout << "inner\n"; });
      std::cout << "outer ends\n";
    });
  std::cout << "after submission, before wait\n";
  pool.wait();
  std::cout << "after wait\n";
}
