#include <iostream>
#include <utility>
#include "simple_thread_pool.hpp"

template <class Executor, class Function>
void defer(const Executor& e, Function f)
{
  e.defer(std::move(f));
}

int count = 0;
simple_thread_pool pool(1);

void foo()
{
  while (++count < 1000000)
    defer(pool.executor(), foo);
}

int main()
{
  defer(pool.executor(), foo);
  pool.join();
  return count;
}
