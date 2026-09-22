#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <iomanip>

struct employee {
    int num;          // идентификационный номер сотрудника
    char name[10];    // имя сотрудника
    double hours;     // количество отработанных часов
};

int main(int argc, char* argv[]) {
    // 1. Проверяем аргументы
    if (argc != 4) {
        std::cerr << "Ошибка: неверное количество аргументов.\n";
        std::cerr << "Использование: ./Reporter <исходный_файл> <файл_отчета> <оплата_за_час>\n";
        return 1;
    }

    std::string bin_filename = argv[1];
    std::string report_filename = argv[2];
    double hourly_rate = std::atof(argv[3]);

    // 2. Открываем файлы
    std::ifstream in(bin_filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Ошибка: не удалось открыть бинарный файл.\n";
        return 1;
    }

    std::ofstream out(report_filename);
    if (!out.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл отчета.\n";
        return 1;
    }

    // 3. Заголовок и шапка отчета
    out << "Отчет по файлу «" << bin_filename << "»\n";
    out << "Номер сотрудника, имя сотрудника, часы, зарплата\n";

    // 4. Построчное чтение и формирование отчета
    employee emp{};
    while (in.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        double salary = emp.hours * hourly_rate;
        out << emp.num << ", " 
            << emp.name << ", " 
            << emp.hours << ", " 
            << std::fixed << std::setprecision(2) << salary << "\n";
    }

    in.close();
    out.close();
    return 0;
}