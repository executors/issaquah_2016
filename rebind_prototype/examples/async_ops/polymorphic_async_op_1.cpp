#include <chrono>
#include <experimental/thread_pool>
#include <functional>
#include <iostream>
#include <thread>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

using task_executor = execution::executor<
        execution::oneway_t,
        execution::single_t,
        execution::never_blocking_t>;

using completion_executor = execution::executor<
        execution::oneway_t,
        execution::single_t,
        execution::never_blocking_t,
        execution::prefer_only<execution::possibly_blocking_t>,
        execution::prefer_only<execution::outstanding_work_t>>;

// An operation that doubles a value asynchronously.
void my_twoway_operation_1(const task_executor& tex, int n,
    const completion_executor& cex, std::function<void(int)> h)
{
  if (n == 0)
  {
    // Nothing to do. Operation finishes immediately.
    // Specify non-blocking to prevent stack overflow.
    cex.require(execution::never_blocking).execute(
        [h = std::move(h), n]() mutable
        {
          h(n);
        });
  }
  else
  {
    // Simulate an asynchronous operation.
    tex.require(execution::never_blocking).execute(
        [n, cex = execution::prefer(cex, execution::outstanding_work), h = std::move(h)]() mutable
        {
          int result = n * 2;
          std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate long running work.
          execution::prefer(cex, execution::possibly_blocking).execute(
              [h = std::move(h), result]() mutable
              {
                h(result);
              });
        });
  }
}

int main()
{
  static_thread_pool task_pool{1};
  static_thread_pool completion_pool{1};
  my_twoway_operation_1(task_pool.executor(), 21, completion_pool.executor(),
      [](int n){ std::cout << "the answer is " << n << "\n"; });
  completion_pool.wait();
}
