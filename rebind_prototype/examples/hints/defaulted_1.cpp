#include <experimental/thread_pool>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_hints
{
  struct tracing { bool on; };

  // Default hint implementation drops it.
  template <class Executor>
    std::enable_if_t<!execution::has_transform_executor_member_v<Executor, tracing>, Executor>
      transform_executor(Executor ex, tracing) { return std::move(ex); }
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

static_assert(execution::is_oneway_executor_v<inline_executor>, "one way executor requirements not met");

int main()
{
  static_thread_pool pool{1};

  auto ex1 = execution::transform_executor(inline_executor(), custom_hints::tracing{true});
  ex1.execute([]{ std::cout << "we made it\n"; });

  auto ex2 = execution::transform_executor(pool.executor(), custom_hints::tracing{true});
  ex2.execute([]{ std::cout << "we made it again\n"; });

  pool.wait();
}
