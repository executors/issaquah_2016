#ifndef STD_EXPERIMENTAL_BITS_TRY_TRANSFORM_EXECUTOR_H
#define STD_EXPERIMENTAL_BITS_TRY_TRANSFORM_EXECUTOR_H

#include <experimental/bits/has_transform_executor_member.h>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

template<class Property> struct is_transform_executor_only;

namespace try_transform_executor_impl {

template<class E> void try_transform_executor(const E&, const oneway_t&) = delete;
template<class E> void try_transform_executor(const E&, const twoway_t&) = delete;
template<class E> void try_transform_executor(const E&, const single_t&) = delete;
template<class E> void try_transform_executor(const E&, const bulk_t&) = delete;

template<class Executor, class Property>
constexpr auto try_transform_executor(Executor&& ex, Property&& p)
  -> decltype(std::forward<Executor>(ex).transform_executor(std::forward<Property>(p)))
{
  return std::forward<Executor>(ex).transform_executor(std::forward<Property>(p));
}

template<class Executor, class Property>
constexpr auto try_transform_executor(Executor ex, Property&&)
  -> typename std::enable_if<!has_transform_executor_member<Executor, Property>::value, Executor>::type
{
  return ex;
}

struct try_transform_executor_fn
{
  template<class Executor, class Property>
  constexpr auto operator()(Executor&& ex, Property&& p) const
    noexcept(noexcept(try_transform_executor(std::forward<Executor>(ex), std::forward<Property>(p))))
    -> decltype(try_transform_executor(std::forward<Executor>(ex), std::forward<Property>(p)))
  {
    return try_transform_executor(std::forward<Executor>(ex), std::forward<Property>(p));
  }

  template<class Executor, class Property0, class Property1, class... PropertyN>
  constexpr auto operator()(Executor&& ex, Property0&& p0, Property1&& p1, PropertyN&&... pn) const
    noexcept(noexcept(std::declval<try_transform_executor_fn>()(std::declval<try_transform_executor_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...)))
    -> decltype(std::declval<try_transform_executor_fn>()(std::declval<try_transform_executor_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...))
  {
    return (*this)((*this)(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...);
  }
};

template<class T = try_transform_executor_fn> constexpr T customization_point{};

} // namespace try_transform_executor_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_TRY_TRANSFORM_EXECUTOR_H
