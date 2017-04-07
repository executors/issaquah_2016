#ifndef INLINE_EXECUTOR_HPP
#define INLINE_EXECUTOR_HPP

class inline_executor
{
public:
  typedef int shape_type;

  template <class Function>
  void execute(Function f) const
  {
    f();
  }

  template <class Function, class SharedFactory>
  void bulk_execute(Function f, shape_type n, SharedFactory sf) const
  {
    auto ss = sf();
    for (shape_type i = 0; i < n; ++i)
    {
      f(i, ss);
    }
  }
};

#endif // INLINE_EXECUTOR_HPP
