#ifndef STD_EXPERIMENTAL_BITS_PREFER_ONLY_H
#define STD_EXPERIMENTAL_BITS_PREFER_ONLY_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

template<class InnerProperty>
struct prefer_only
{
  InnerProperty property;

  static constexpr bool is_requirable = false;
  static constexpr bool is_preferable = InnerProperty::is_preferable;

  using polymorphic_query_result_type =
    typename InnerProperty::polymorphic_query_result_type;

  template<class Executor, class Type = decltype(InnerProperty::template static_query_v<Executor>)>
    static constexpr Type static_query_v = InnerProperty::template static_query_v<Executor>;

  constexpr prefer_only(const InnerProperty& p) : property(p) {}

  constexpr auto value() const
    noexcept(noexcept(std::declval<const InnerProperty>().value()))
      -> decltype(std::declval<const InnerProperty>().value())
  {
    return property.value();
  }

  template<class Executor, class Property, class = typename std::enable_if<std::is_same<Property, prefer_only>::value>::type>
  friend auto prefer(Executor ex, const Property& p)
    noexcept(noexcept(execution::prefer(std::move(ex), std::declval<const InnerProperty>())))
      -> decltype(execution::prefer(std::move(ex), std::declval<const InnerProperty>()))
  {
    return execution::prefer(std::move(ex), p.property);
  }

  template<class Executor, class Property, class = typename std::enable_if<std::is_same<Property, prefer_only>::value>::type>
  friend constexpr auto query(const Executor& ex, const Property& p)
    noexcept(noexcept(execution::query(ex, std::declval<const InnerProperty>())))
    -> decltype(execution::query(ex, std::declval<const InnerProperty>()))
  {
    return execution::query(ex, p.property);
  }
};

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_PREFER_ONLY_H
