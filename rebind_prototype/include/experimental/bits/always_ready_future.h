#ifndef STD_EXPERIMENTAL_BITS_ALWAYS_READY_FUTURE_H
#define STD_EXPERIMENTAL_BITS_ALWAYS_READY_FUTURE_H

namespace std {
namespace experimental {
inline namespace concurrency_v2 {
namespace execution {
namespace impl {

template<class T>
class always_ready_future
{
  public:
    always_ready_future(const T& value) : value_or_exception_(value) {}

    always_ready_future(T&& value) : value_or_exception_(std::move(value)) {}
    
    //always_ready_future(std::exception_ptr e) : value_e() {}

    T get()
    {
      return std::move(value_or_exception_);
    }

    void wait() const {}

  private:
    // XXX should be variant<T,exception_ptr>
    T value_or_exception_;
};

template<>
class always_ready_future<void>
{
  public:
    always_ready_future() {}
    
    //always_ready_future(std::exception_ptr e) : maybe_exception_(e) {}

    void get() const {}

    void wait() const {}

  private:
    // XXX should be optional<exception_ptr>
};

template<class T>
always_ready_future<typename std::decay<T>::type> make_always_ready_future(T&& value)
{
  return always_ready_future<typename std::decay<T>::type>(std::forward<T>(value));
}

inline always_ready_future<void> make_always_ready_future()
{
  return always_ready_future<void>();
}

} // namespace impl
} // namespace execution
} // inline namespace concurrency_v2
} // namespace experimental
} // namespace std

#endif // STD_EXPERIMENTAL_BITS_ALWAYS_READY_FUTURE_H

