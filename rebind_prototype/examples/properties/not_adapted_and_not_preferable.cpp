#include <experimental/thread_pool>
#include <cassert>
#include <iostream>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;

namespace custom_props
{
  struct tracing
  {
    static constexpr bool is_requirable = true;
    static constexpr bool is_preferable = false;
    using polymorphic_query_result_type = bool;

    bool on = false;
  };
};

class inline_executor
{
public:
  inline_executor require(custom_props::tracing t) const { inline_executor tmp(*this); tmp.tracing_ = t.on; return tmp; }

  bool query(custom_props::tracing) const { return tracing_; }

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

  auto ex1 = execution::require(inline_executor(), custom_props::tracing{true});
  assert(execution::query(ex1, custom_props::tracing{}));
  ex1.execute([]{ std::cout << "we made it\n"; });

  static_assert(!execution::can_prefer_v<inline_executor, custom_props::tracing>, "cannot prefer");

  // No adaptation means we can't require arbitrary executors using our custom property ...
  static_assert(!execution::can_require_v<static_thread_pool::executor_type, custom_props::tracing>, "can't require tracing from static_thread_pool");
  static_assert(!execution::can_query_v<static_thread_pool::executor_type, custom_props::tracing>, "can't query tracing from static_thread_pool");

  // ... and we can't ask for it as a preference either.
  static_assert(!execution::can_prefer_v<static_thread_pool::executor_type, custom_props::tracing>, "cannot prefer");
}
