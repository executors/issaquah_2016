#ifndef STD_EXPERIMENTAL_BITS_IS_BULK_ONE_WAY_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_IS_BULK_ONE_WAY_EXECUTOR_H

#include <experimental/bits/is_executor.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace is_bulk_one_way_executor_impl {

template<class...>
struct type_check
{
  typedef void type;
};

struct shared_state {};

struct bulk_function
{
  void operator()(std::size_t, shared_state&) {}
};

template<class T, class = void>
struct eval : std::false_type {};

template<class T>
struct eval<T,
  typename type_check<
    typename std::enable_if<std::is_same<void, decltype(std::declval<const T&>()(std::declval<bulk_function>(), 1, std::declval<shared_state>()))>::value>::type,
    typename std::enable_if<std::is_same<void, decltype(std::declval<const T&>()(std::declval<bulk_function&>(), 1, std::declval<shared_state>()))>::value>::type,
    typename std::enable_if<std::is_same<void, decltype(std::declval<const T&>()(std::declval<const bulk_function&>(), 1, std::declval<shared_state>()))>::value>::type,
    typename std::enable_if<std::is_same<void, decltype(std::declval<const T&>()(std::declval<bulk_function&&>(), 1, std::declval<shared_state>()))>::value>::type
	>::type> : is_executor<T> {};

} // namespace is_bulk_one_way_executor_impl

template<class Executor>
struct is_bulk_one_way_executor : is_bulk_one_way_executor_impl::eval<Executor> {};

} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_IS_BULK_ONE_WAY_EXECUTOR_H