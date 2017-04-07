#include <iostream>
#include <utility>
#include "inline_executor.hpp"

template <class Executor, class Function>
void execute(const Executor& e, Function f)
{
  e.execute(std::move(f));
}

int count = 0;

void __attribute__((__noinline__)) foo()
{
  ++count;
}

int main()
{
  inline_executor ex;
  for (int i = 0; i < 1000000000; ++i)
    execute(ex, foo);
  return count;
}
