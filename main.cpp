// Лабораторная работа 3: main + несколько потоков marker.
// Сборка: g++ -std=c++17 -pthread main.cpp -o lab3
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "sharedarray.h"

using namespace std;

enum class Cmd { Run, Wait, Stop };

// "События" (аналог Event из WinAPI) = мьютекс + условная переменная + флаг.
struct Control {
    mutex m;
    condition_variable cv;
    bool started = false;      // событие "начать работу" (сигнал от main)
    int blocked = 0;           // сколько marker-потоков сообщили "не могу продолжать"
    vector<Cmd> cmd;           // индивидуальная команда для каждого marker
};

mutex print_mtx;  // чтобы вывод разных потоков не перемешивался

void print_array(const vector<int>& a) {
    lock_guard<mutex> lk(print_mtx);
    for (int v : a) cout << v << ' ';
    cout << endl;
}

void marker(SharedArray& arr, Control& ctl, int id) {
    // 1. Ждём сигнал начала от main
    {
        unique_lock<mutex> lk(ctl.m);
        ctl.cv.wait(lk, [&] { return ctl.started; });
    }

    // 2. srand(id). rand() общий для всех потоков, поэтому берём rand_r с личным seed
    unsigned seed = id;
    int marked = 0;

    while (true) {
        // 3.1-3.2
        size_t idx = rand_r(&seed) % arr.size();

        // 3.3 свободен -> помечаем (спим 5 мс до и после внутри try_mark)
        if (arr.try_mark(idx, id, chrono::milliseconds(5))) {
            ++marked;
            continue;
        }

        // 3.4 занят -> сообщаем информацию и ждём команду main
        {
            lock_guard<mutex> lk(print_mtx);
            cout << "marker " << id << ": помечено " << marked
                 << ", не могу пометить индекс " << idx << endl;
        }
        Cmd c;
        {
            unique_lock<mutex> lk(ctl.m);
            ctl.cmd[id] = Cmd::Wait;
            ++ctl.blocked;
            ctl.cv.notify_all();                         // сигнал main
            ctl.cv.wait(lk, [&] { return ctl.cmd[id] != Cmd::Wait; });
            c = ctl.cmd[id];
        }
        if (c == Cmd::Stop) {          // 4. завершение
            arr.clear_marked(id);
            return;
        }
        // 5. иначе продолжаем цикл
    }
}

int main() {
    // 1-2. массив нулей
    size_t n;
    cout << "Размер массива: ";
    cin >> n;
    SharedArray arr(n);

    // 3. количество потоков
    int count;
    cout << "Количество marker-потоков: ";
    cin >> count;

    Control ctl;
    ctl.cmd.assign(count + 1, Cmd::Wait);   // нумерация с 1

    // 4. запуск
    vector<thread> threads(count + 1);
    vector<bool> alive(count + 1, true);
    for (int i = 1; i <= count; ++i)
        threads[i] = thread(marker, ref(arr), ref(ctl), i);

    // 5. сигнал начала
    {
        lock_guard<mutex> lk(ctl.m);
        ctl.started = true;
    }
    ctl.cv.notify_all();

    // 6. основной цикл
    for (int left = count; left > 0; --left) {
        // 6.1 ждём, пока все живые marker подадут сигнал
        {
            unique_lock<mutex> lk(ctl.m);
            ctl.cv.wait(lk, [&] { return ctl.blocked == left; });
        }
        // 6.2
        cout << "Массив: ";
        print_array(arr.snapshot());

        // 6.3
        int k;
        while (true) {
            cout << "Какой marker завершить? ";
            if (!(cin >> k)) return 0;
            if (k >= 1 && k <= count && alive[k]) break;
            cout << "Нет такого активного потока\n";
        }

        // 6.4 сигнал на завершение
        {
            lock_guard<mutex> lk(ctl.m);
            ctl.cmd[k] = Cmd::Stop;
        }
        ctl.cv.notify_all();

        // 6.5 ждём завершения
        threads[k].join();
        alive[k] = false;

        // 6.6
        cout << "Массив: ";
        print_array(arr.snapshot());

        // 6.7 сигнал "продолжайте" остальным
        {
            lock_guard<mutex> lk(ctl.m);
            ctl.blocked = 0;
            for (int i = 1; i <= count; ++i)
                if (alive[i]) ctl.cmd[i] = Cmd::Run;
        }
        ctl.cv.notify_all();
    }
    cout << "Все потоки завершены." << endl;
    return 0;
}