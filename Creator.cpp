#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

struct employee {
    int num;          // идентификационный номер сотрудника
    char name[10];    // имя сотрудника
    double hours;     // количество отработанных часов
};

int main(int argc, char* argv[]) {
    // 1. Проверяем аргументы командной строки
    if (argc != 3) {
        std::cerr << "Ошибка: неверное количество аргументов.\n";
        std::cerr << "Использование: ./Creator <имя_файла> <количество_записей>\n";
        return 1;
    }

    std::string filename = argv[1];
    int count = std::atoi(argv[2]);

    // 2. Открываем бинарный файл для записи
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Ошибка: не удалось создать бинарный файл.\n";
        return 1;
    }

    // 3. Вводим записи с консоли и сохраняем
    for (int i = 0; i < count; ++i) {
        employee emp{};
        std::cout << "Запись " << (i + 1) << " из " << count << ":\n";
        std::cout << "  Введите ID (число): ";
        std::cin >> emp.num;
        std::cout << "  Введите имя (до 9 символов): ";
        std::cin >> emp.name;
        std::cout << "  Введите часы: ";
        std::cin >> emp.hours;

        out.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
    }

    out.close();
    return 0;
}