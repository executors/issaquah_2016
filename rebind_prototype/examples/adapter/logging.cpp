#include <experimental/thread_pool>
#include <iostream>
#include <memory>
#include <string>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;
using std::experimental::concurrency_v2::future;

template <class InnerExecutor>
class logging_executor
{
  std::shared_ptr<std::string> prefix_;
  InnerExecutor inner_ex_;

  template <class T> static auto inner_declval() -> decltype(std::declval<InnerExecutor>());

  template <class Function>
  auto wrap(Function f) const
  {
    return [prefix = *prefix_, f = std::move(f)]() mutable
        {
          std::cout << prefix << ": " << "function begins\n";

          struct on_exit
          {
            std::string& prefix;
            ~on_exit() { std::cout << prefix << ": " << "function ends\n"; }
          } x{prefix};

          return f();
        };
  }

public:
  logging_executor(const std::string& prefix, const InnerExecutor& ex)
    : prefix_(std::make_shared<std::string>(prefix)), inner_ex_(ex) {}

  template <class Property> auto transform_executor(const Property& p) const &
    -> logging_executor<execution::transform_executor_member_result_t<InnerExecutor, Property>>
      { return { *prefix_, inner_ex_.transform_executor(p) }; }
  template <class Property> auto transform_executor(const Property& p) &&
    -> logging_executor<execution::transform_executor_member_result_t<InnerExecutor, Property>>
      { return { *prefix_, std::move(inner_ex_).transform_executor(p) }; }

  auto& context() const noexcept { return inner_ex_.context(); }

  friend bool operator==(const logging_executor& a, const logging_executor& b) noexcept
  {
    return *a.prefix_ == *b.prefix_ && a.inner_ex_ == b.inner_ex_;
  }

  friend bool operator!=(const logging_executor& a, const logging_executor& b) noexcept
  {
    return !(a == b);
  }

  template <class Function>
  auto execute(Function f) const
    -> decltype(inner_declval<Function>().execute(std::move(f)))
  {
    return inner_ex_.execute(this->wrap(std::move(f)));
  }

  template <class Function>
  auto twoway_execute(Function f) const
    -> decltype(inner_declval<Function>().twoway_execute(std::move(f)))
  {
    return inner_ex_.twoway_execute(this->wrap(std::move(f)));
  }
};

static_assert(execution::is_oneway_executor_v<
  logging_executor<static_thread_pool::executor_type>>,
    "one way executor requirements must be met");
static_assert(execution::is_oneway_executor_v<
  logging_executor<static_thread_pool::executor_type>>,
    "two way executor requirements must be met");

int main()
{
  static_thread_pool pool{1};
  logging_executor<static_thread_pool::executor_type> ex1("LOG", pool.executor());
  ex1.execute([]{ std::cout << "we made it\n"; });
  auto ex2 = ex1.transform_executor(execution::always_blocking);
  ex2.execute([]{ std::cout << "we made it again\n"; });
  auto ex3 = ex2.transform_executor(execution::never_blocking).transform_executor(execution::continuation);
  ex3.execute([]{ std::cout << "and again\n"; });
  auto ex4 = ex1.transform_executor(execution::twoway);
  future<int> f = ex4.twoway_execute([]{ std::cout << "computing result\n"; return 42; });
  pool.wait();
  std::cout << "result is " << f.get() << "\n";
}
