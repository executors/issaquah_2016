#include <experimental/execution>
#include <experimental/thread_pool>

namespace execution = std::experimental::execution;
using std::experimental::static_thread_pool;
using std::experimental::executors_v1::future;

using executor = execution::executor<
    execution::oneway_t,
    execution::twoway_t,
    execution::single_t,
    execution::bulk_t,
    execution::thread_execution_mapping_t,
    execution::never_blocking_t,
    execution::possibly_blocking_t,
    execution::always_blocking_t
  >;

void executor_compile_test()
{
  static_assert(execution::is_oneway_executor_v<executor>, "is_oneway_executor must evaluate true");
  static_assert(execution::is_twoway_executor_v<executor>, "is_twoway_executor must evaluate true");

  static_thread_pool pool(0);

  static_assert(noexcept(executor()), "default constructor must not throw");
  static_assert(noexcept(executor(nullptr)), "nullptr constructor must not throw");

  executor ex1;
  executor ex2(nullptr);

  const executor& cex1 = ex1;
  const executor& cex2 = ex2;

  static_assert(noexcept(executor(cex1)), "copy constructor must not throw");
  static_assert(noexcept(executor(std::move(ex1))), "move constructor must not throw");

  executor ex3(ex1);
  executor ex4(std::move(ex1));

  executor ex5(pool.executor());

  execution::executor<> ex6(ex5);
  execution::executor<execution::oneway_t> ex7(ex5);
  execution::executor<execution::oneway_t, execution::single_t> ex8(ex5);

  static_assert(noexcept(ex2 = cex1), "copy assignment must not throw");
  static_assert(noexcept(ex3 = std::move(ex1)), "move assignment must not throw");
  static_assert(noexcept(ex3 = nullptr), "nullptr assignment must not throw");

  ex2 = ex1;
  ex3 = std::move(ex1);
  ex4 = nullptr;
  ex5 = pool.executor();

  static_assert(noexcept(ex1.swap(ex2)), "swap must not throw");

  ex1.swap(ex2);

  ex1.assign(pool.executor());

  ex1 = execution::require(cex1, execution::oneway);
  ex1 = execution::require(cex1, execution::twoway);
  ex1 = execution::require(cex1, execution::single);
  ex1 = execution::require(cex1, execution::bulk);
  ex1 = execution::require(cex1, execution::thread_execution_mapping);
  ex1 = execution::require(cex1, execution::never_blocking);
  ex1 = execution::require(cex1, execution::possibly_blocking);
  ex1 = execution::require(cex1, execution::always_blocking);

  ex1 = execution::prefer(cex1, execution::thread_execution_mapping);
  ex1 = execution::prefer(cex1, execution::never_blocking);
  ex1 = execution::prefer(cex1, execution::possibly_blocking);
  ex1 = execution::prefer(cex1, execution::always_blocking);
  ex1 = execution::prefer(cex1, execution::continuation);
  ex1 = execution::prefer(cex1, execution::not_continuation);
  ex1 = execution::prefer(cex1, execution::outstanding_work);
  ex1 = execution::prefer(cex1, execution::not_outstanding_work);
  ex1 = execution::prefer(cex1, execution::bulk_sequenced_execution);
  ex1 = execution::prefer(cex1, execution::bulk_parallel_execution);
  ex1 = execution::prefer(cex1, execution::bulk_unsequenced_execution);
  ex1 = execution::prefer(cex1, execution::new_thread_execution_mapping);

  cex1.execute([]{});

  std::experimental::executors_v1::future<int> f1 = cex1.twoway_execute([]{ return 42; });
  (void)f1;

  std::experimental::executors_v1::future<void> f2 = cex1.twoway_execute([]{});
  (void)f2;

  cex1.bulk_execute([](std::size_t, int&){}, 1, []{ return 42; });

  bool b1 = static_cast<bool>(ex1);
  (void)b1;

  const std::type_info& target_type = cex1.target_type();
  (void)target_type;

  static_thread_pool::executor_type* ex9 = ex1.target<static_thread_pool::executor_type>();
  (void)ex9;

  const static_thread_pool::executor_type* cex6 = ex1.target<static_thread_pool::executor_type>();
  (void)cex6;

  bool b2 = (cex1 == cex2);
  (void)b2;

  bool b3 = (cex1 != cex2);
  (void)b3;

  bool b4 = (cex1 == nullptr);
  (void)b4;

  bool b5 = (cex1 != nullptr);
  (void)b5;

  bool b6 = (nullptr == cex2);
  (void)b6;

  bool b7 = (nullptr != cex2);
  (void)b7;

  swap(ex1, ex2);
}

int main()
{
}
