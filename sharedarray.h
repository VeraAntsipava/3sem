#pragma once
// Общий массив, защищённый мьютексом (аналог CRITICAL_SECTION из WinAPI).
#include <chrono>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

class SharedArray {
public:
    explicit SharedArray(std::size_t n) : data_(n, 0) {}

    std::size_t size() const { return data_.size(); }

    // Пытается пометить элемент номером id.
    // Если элемент свободен (== 0) -> спим, пишем id, спим, возвращаем true.
    // Если занят -> возвращаем false.
    // Проверка и запись выполняются под одним захватом мьютекса (критическая секция).
    bool try_mark(std::size_t idx, int id,
                  std::chrono::milliseconds pause = std::chrono::milliseconds(0)) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (data_[idx] != 0) return false;
        if (pause.count() > 0) std::this_thread::sleep_for(pause);
        data_[idx] = id;
        if (pause.count() > 0) std::this_thread::sleep_for(pause);
        return true;
    }

    // Обнуляет все элементы, помеченные номером id. Возвращает, сколько обнулили.
    int clear_marked(int id) {
        std::lock_guard<std::mutex> lock(mtx_);
        int cleared = 0;
        for (auto& v : data_)
            if (v == id) { v = 0; ++cleared; }
        return cleared;
    }

    int count_marked(int id) const {
        std::lock_guard<std::mutex> lock(mtx_);
        int c = 0;
        for (int v : data_) if (v == id) ++c;
        return c;
    }

    std::vector<int> snapshot() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_;
    }

private:
    mutable std::mutex mtx_;
    std::vector<int> data_;
};