#ifndef STD_EXPERIMENTAL_BITS_TRANSFORM_H
#define STD_EXPERIMENTAL_BITS_TRANSFORM_H

#include <experimental/bits/has_transform_member.h>
#include <experimental/bits/is_twoway_executor.h>
#include <experimental/bits/is_oneway_executor.h>
#include <experimental/bits/is_bulk_oneway_executor.h>
#include <experimental/bits/is_bulk_twoway_executor.h>
#include <utility>

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {

struct oneway_t;
struct twoway_t;
struct single_t;
struct bulk_t;
struct always_blocking_t;
struct possibly_blocking_t;
struct continuation_t;
struct not_continuation_t;
struct outstanding_work_t;
struct not_outstanding_work_t;
template<class> struct allocator_t;

namespace transform_impl {

template<class Executor, class Property>
constexpr auto transform(Executor&& ex, Property&& p)
  -> decltype(std::forward<Executor>(ex).transform(std::forward<Property>(p)))
{
  return std::forward<Executor>(ex).transform(std::forward<Property>(p));
}

// Forward declare the default adaptations.
template<class Executor>
  constexpr typename std::enable_if<
    (is_oneway_executor<Executor>::value || is_bulk_oneway_executor<Executor>::value)
    && !has_transform_member<Executor, oneway_t>::value, Executor>::type
      transform(Executor ex, oneway_t);
template<class Executor> class twoway_adapter;
template<class Executor>
  typename std::enable_if<
    (is_oneway_executor<Executor>::value || is_bulk_oneway_executor<Executor>::value)
    && !(is_twoway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_transform_member<Executor, twoway_t>::value, twoway_adapter<Executor>>::type
      transform(Executor ex, twoway_t);
template<class Executor> class bulk_adapter;
template<class Executor>
  constexpr typename std::enable_if<
    (is_twoway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_transform_member<Executor, twoway_t>::value, Executor>::type
      transform(Executor ex, twoway_t);
template<class Executor>
  typename std::enable_if<is_oneway_executor<Executor>::value
    && !(is_bulk_oneway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_transform_member<Executor, bulk_t>::value, bulk_adapter<Executor>>::type
      transform(Executor ex, bulk_t);
template<class Executor>
  constexpr typename std::enable_if<
    (is_bulk_oneway_executor<Executor>::value || is_bulk_twoway_executor<Executor>::value)
    && !has_transform_member<Executor, bulk_t>::value, Executor>::type
      transform(Executor ex, bulk_t);
template<class Executor> class always_blocking_adapter;
template<class Executor>
  constexpr typename std::enable_if<!has_transform_member<Executor, always_blocking_t>::value,
    always_blocking_adapter<Executor>>::type
      transform(Executor ex, always_blocking_t);
template<class Executor>
  constexpr typename std::enable_if<!has_transform_member<Executor, possibly_blocking_t>::value, Executor>::type
    transform(Executor ex, possibly_blocking_t);

struct transform_fn
{
  template<class Executor, class Property>
  constexpr auto operator()(Executor&& ex, Property&& p) const
    noexcept(noexcept(transform(std::forward<Executor>(ex), std::forward<Property>(p))))
    -> decltype(transform(std::forward<Executor>(ex), std::forward<Property>(p)))
  {
    return transform(std::forward<Executor>(ex), std::forward<Property>(p));
  }

  template<class Executor, class Property0, class Property1, class... PropertyN>
  constexpr auto operator()(Executor&& ex, Property0&& p0, Property1&& p1, PropertyN&&... pn) const
    noexcept(noexcept(std::declval<transform_fn>()(std::declval<transform_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...)))
    -> decltype(std::declval<transform_fn>()(std::declval<transform_fn>()(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...))
  {
    return (*this)((*this)(std::forward<Executor>(ex), std::forward<Property0>(p0)), std::forward<Property1>(p1), std::forward<PropertyN>(pn)...);
  }
};

template<class T = transform_fn> constexpr T customization_point{};

} // namespace transform_impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_TRANSFORM_H
