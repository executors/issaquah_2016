#include <experimental/execution>
#include <iostream>

namespace execution = std::experimental::execution;

class inline_executor
{
public:
  inline_executor transform(execution::always_blocking_t) const { return *this; }
  inline_executor transform(execution::possibly_blocking_t) const { return *this; }

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
    f();
  }
};

static_assert(execution::is_oneway_executor_v<inline_executor>, "one way executor requirements not met");

int main()
{
  inline_executor ex1;
  auto ex2 = ex1.transform(execution::always_blocking);
  ex2.execute([]{ std::cout << "we made it\n"; });
}
