#ifndef STD_EXPERIMENTAL_BITS_CAN_TRANSFORM_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_CAN_TRANSFORM_EXECUTOR_H

#include <type_traits>
#include <tuple>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace can_transform_executor_impl {

template<class>
struct type_check
{
  typedef void type;
};

template<class Executor, class Properties, class = void>
struct eval : std::false_type {};

template<class Executor, class... Properties>
struct eval<Executor, std::tuple<Properties...>,
  typename type_check<decltype(
    ::std::experimental::concurrency_v2::execution::transform_executor(std::declval<Executor>(), std::declval<Properties>()...)
  )>::type> : std::true_type {};

} // namespace can_transform_executor_impl

template<class Executor, class... Properties>
struct can_transform_executor : can_transform_executor_impl::eval<Executor, std::tuple<Properties...>> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_CAN_TRANSFORM_EXECUTOR_H
