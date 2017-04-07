#include <iostream>
#include <utility>
#include "simple_thread_pool.hpp"

template <class Executor, class Function>
void post(const Executor& e, Function f)
{
  e.post(std::move(f));
}

int count = 0;
simple_thread_pool pool(1);

void foo()
{
  while (++count < 1000000)
    post(pool.executor(), foo);
}

int main()
{
  post(pool.executor(), foo);
  pool.join();
  return count;
}
