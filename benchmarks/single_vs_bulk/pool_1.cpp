#include <iostream>
#include <utility>
#include "simple_thread_pool.hpp"

template <class Executor, class Function>
void execute(const Executor& e, Function f)
{
  e.execute(std::move(f));
}

int count = 0;
simple_thread_pool pool(1);

void foo()
{
  while (++count < 1000000)
    execute(pool.executor(), foo);
}

int main()
{
  execute(pool.executor(), foo);
  pool.join();
  return count;
}
