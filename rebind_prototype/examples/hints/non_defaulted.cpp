#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  struct tracing { bool on; };
};

class inline_executor
{
public:
  inline_executor transform_executor(custom_hints::tracing t) const { inline_executor tmp(*this); tmp.tracing_ = t.on; return tmp; }

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
  void execute(Function f) const noexcept
  {
    if (tracing_) std::cout << "running function inline\n";
    f();
  }

private:
  bool tracing_;
};

static_assert(execution::is_oneway_executor_v<inline_executor>, "one way executor transform_executorments not met");

int main()
{
  static_thread_pool pool{1};

  auto ex1 = execution::transform_executor(inline_executor(), custom_hints::tracing{true});
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::try_transform_executor(inline_executor(), custom_hints::tracing{true});
  ex2.execute([]{ std::cout << "we made it with a try_transform_executorence\n"; });

  // No default means we can't transform_executor arbitrary executors using our custom hint ...
  static_assert(!execution::can_transform_executor_v<static_thread_pool::executor_type, custom_hints::tracing>, "can't transform_executor tracing from static_thread_pool");

  // ... but we can still ask for it as a try_transform_executorence.
  auto ex3 = execution::try_transform_executor(pool.executor(), custom_hints::tracing{true});
  ex3.execute([]{ std::cout << "we made it again with a try_transform_executorence\n"; });
  pool.wait();
}
