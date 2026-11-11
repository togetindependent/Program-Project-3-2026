#include <iostream>
#include <thread>
#include <vector>
#include <syncstream>
#include <chrono>
#include <semaphore>
#include <latch>
#include <barrier>

using namespace std;

// Заглушка згідно з умовою: виклик f(x, i) з засинанням на 1 секунду
void f(char x, int i)
{
    // Симуляція виконання дії тривалістю 1 секунда
    this_thread::sleep_for(1s);
    osyncstream(cout) << format("З набору {} виконано дію {}.\n", x, i);
}

// Примітиви синхронізації для зв'язків між гілками графа №3
// Згідно зі структурою графа №3:
// 1. Набір 'a' виконується першим. Після нього стартують паралельні гілки b, c, d.
// 2. Гілка 'e' залежить від 'b' та 'g' (але 'g' залежить від 'c'). 
//    Тому 'e' чекає на завершення 'b' та 'g'.
// 3. Гілка 'g' та 'h' залежать від 'c'.
// 4. Гілка 'i' залежить від 'd'.
// 5. Гілка 'j' залежить від 'e' та 'g'.
// 6. Гілка 'k' залежить від 'h' та 'i'.

latch after_a(1);
latch after_b(1);
latch after_c(1);
latch after_d(1);
latch after_g(1);
latch after_h(1);
latch after_i(1);
latch after_e(1);

// Функція для виконання певної кількості дій набору в одному потоці
void run_tasks(char name, int start_idx, int count)
{
    for (int i = 0; i < count; ++i)
    {
        f(name, start_idx + i);
    }
}

int main(int argc, char* argv[])
{
    system("chcp 65001 > nul");
    // Виведення початку обчислення
    cout << "Обчислення розпочато." << endl;
    auto start_time = chrono::steady_clock::now();

    // Запускаємо рівно nt = 7 потоків (плюс головний main), розподіляючи навантаження hardcoded-методом
    // Набір а = 7 дій, b = 8, c = 8, d = 9, e = 8, g = 5, h = 4, i = 8, j = 8, k = 4

    {
    jthread t1([&]() {
        // Потік 1: виконує повністю набір 'a' (7 дій), потім запускає 'b' (8 дій)
        run_tasks('a', 1, 7);
        after_a.count_down();

        after_a.wait();
        run_tasks('b', 1, 8);
        after_b.count_down();
        });

    jthread t2([&]() {
        // Потік 2: чекає ' а', виконує набір 'c' (8 дій)
        after_a.wait();
        run_tasks('c', 1, 8);
        after_c.count_down();
        });

    jthread t3([&]() {
        // Потік 3: чекає ' а', виконує набір 'd' (9 дій)
        after_a.wait();
        run_tasks('d', 1, 9);
        after_d.count_down();
        });

    jthread t4([&]() {
        // Потік 4: чекає 'c', виконує набір 'g' (5 дій)
        after_c.wait();
        run_tasks('g', 1, 5);
        after_g.count_down();
        });

    jthread t5([&]() {
        // Потік 5: чекає 'c', виконує набір 'h' (4 дії)
        after_c.wait();
        run_tasks('h', 1, 4);
        after_h.count_down();
        });

    jthread t6([&]() {
        // Потік 6: чекає 'd', виконує набір 'i' (8 дій)
        after_d.wait();
        run_tasks('i', 1, 8);
        after_i.count_down();
        });

    jthread t7([&]() {
        // Потік 7: обробляє складніші залежні вузли (e, j, k)
        // Чекає на 'b' та 'g', щоб виконати 'e' (8 дій)
        after_b.wait();
        after_g.wait();
        run_tasks('e', 1, 8);
        after_e.count_down();

        // Також потік 7 дочікується 'e' та 'g' для фінального 'j' (8 дій) та 'h'/'i' для 'k' (4 дії)
        after_e.wait();
        run_tasks('j', 1, 8);

        after_h.wait();
        after_i.wait();
        run_tasks('k', 1, 4);
        });
    }

    // Деструктори jthread автоматично зроблять join при виході з блоку main, 
    // гарантуючи завершення усіх 7 потоків.

    auto end_time = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    // Виведення повідомлення про завершення з тривалістю
    cout << format("Обчислення завершено. Тривалість обчислення {} секунд.", duration) << endl;

    return 0;
}