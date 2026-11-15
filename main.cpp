//Zhylinskyi Oleksandr K-24

#include <iostream>
#include <thread>
#include <vector>
#include <syncstream>
#include <chrono>
#include <semaphore>
#include <latch>
#include <barrier>
#include <format>

using namespace std;

void f(char x, int i)
{
    this_thread::sleep_for(1s);
    osyncstream(cout) << format("З набору {} виконано дію {}.\n", x, i);
}


latch after_a(1);
latch after_b(1);
latch after_c(1);
latch after_d(1);
latch after_g(1);
latch after_h(1);
latch after_i(1);
latch after_e(1);

void run_tasks(char name, int start_id, int cnt)
{
    for (int i = 0; i < cnt; ++i)
    {
        f(name, start_id + i);
    }
}

int main(int argc, char* argv[])
{
    system("chcp 65001 > nul");

    osyncstream(cout) << "Обчислення розпочато.\n";
    auto start_time = chrono::steady_clock::now();

    {
        jthread t1([&]() {
            run_tasks('a', 1, 7);
            after_a.count_down();

            after_a.wait();
            run_tasks('b', 1, 8);
            after_b.count_down();
            });

        jthread t2([&]() {
            after_a.wait();
            run_tasks('c', 1, 8);
            after_c.count_down();
            });

        jthread t3([&]() {
            after_a.wait();
            run_tasks('d', 1, 9);
            after_d.count_down();
            });

        jthread t4([&]() {
            after_c.wait();
            run_tasks('g', 1, 5);
            after_g.count_down();
            });

        jthread t5([&]() {
            after_c.wait();
            run_tasks('h', 1, 4);
            after_h.count_down();
            });

        jthread t6([&]() {
            after_d.wait();
            run_tasks('i', 1, 8);
            after_i.count_down();
            });

        jthread t7([&]() {
            after_b.wait();
            after_g.wait();

            run_tasks('e', 1, 8);
            after_e.count_down();

            after_e.wait();
            run_tasks('j', 1, 8);

            after_h.wait();
            after_i.wait();
            run_tasks('k', 1, 4);
            });
    }

    auto end_time = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    osyncstream(cout) << format("Обчислення завершено. Тривалість обчислення {} секунд.\n", duration);

    return 0;
}