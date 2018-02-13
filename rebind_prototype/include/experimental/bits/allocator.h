#ifndef STD_EXPERIMENTAL_BITS_ALLOCATOR_H
#define STD_EXPERIMENTAL_BITS_ALLOCATOR_H

namespace std {
namespace experimental {
inline namespace executors_v1 {
namespace execution {

struct default_allocator_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;
};

constexpr default_allocator_t default_allocator;

template<class ProtoAllocator>
struct allocator_t
{
  static constexpr bool is_requirable = true;
  static constexpr bool is_preferable = true;

  ProtoAllocator alloc;
};

template<>
struct allocator_t<void>
{
  static constexpr bool is_requirable = false;
  static constexpr bool is_preferable = false;

  template<class ProtoAllocator>
  allocator_t<ProtoAllocator> operator()(const ProtoAllocator& a) const
  {
    return {a};
  }
};

constexpr allocator_t<void> allocator;

} // namespace execution
} // inline namespace executors_v1
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_ALLOCATOR_H
