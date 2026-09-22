#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

struct employee {
    int num;
    char name[10];
    double hours;
};

// Функция вывода бинарного файла на консоль
void printBinaryFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Ошибка чтения бинарного файла.\n";
        return;
    }

    std::cout << "\n=== Содержимое бинарного файла («" << filename << "») ===\n";
    employee emp{};
    while (in.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        std::cout << "ID: " << emp.num 
                  << " | Имя: " << emp.name 
                  << " | Часы: " << emp.hours << "\n";
    }
    std::cout << "================================================\n\n";
    in.close();
}

// Функция вывода текстового отчета на консоль
void printReportFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Ошибка чтения файла отчета.\n";
        return;
    }

    std::cout << "\n=== Сформированный отчет («" << filename << "») ===\n";
    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << "\n";
    }
    std::cout << "===========================================\n\n";
    in.close();
}

int main() {
    std::string bin_filename;
    std::string count_str;

    // 1. Запрос данных для Creator
    std::cout << "Введите имя бинарного файла: ";
    std::cin >> bin_filename;
    std::cout << "Введите количество записей: ";
    std::cin >> count_str;

    // 2 & 3. Запуск Creator (fork + execl) и ожидание (waitpid)
    pid_t pid_creator = fork();
    if (pid_creator == 0) {
        // Дочерний процесс
        execl("./Creator", "Creator", bin_filename.c_str(), count_str.c_str(), nullptr);
        std::cerr << "Ошибка вызова execl для Creator.\n";
        return 1;
    } else if (pid_creator > 0) {
        // Родительский процесс ждет завершения
        int status;
        waitpid(pid_creator, &status, 0);
    } else {
        std::cerr << "Ошибка создания процесса (fork).\n";
        return 1;
    }

    // 4. Вывод бинарного файла
    printBinaryFile(bin_filename);

    // 5. Запрос данных для Reporter
    std::string report_filename;
    std::string wage_str;
    std::cout << "Введите имя файла отчета: ";
    std::cin >> report_filename;
    std::cout << "Введите оплату за час работы: ";
    std::cin >> wage_str;

    // 6 & 7. Запуск Reporter и ожидание
    pid_t pid_reporter = fork();
    if (pid_reporter == 0) {
        execl("./Reporter", "Reporter", bin_filename.c_str(), report_filename.c_str(), wage_str.c_str(), nullptr);
        std::cerr << "Ошибка вызова execl для Reporter.\n";
        return 1;
    } else if (pid_reporter > 0) {
        int status;
        waitpid(pid_reporter, &status, 0);
    } else {
        std::cerr << "Ошибка создания процесса (fork).\n";
        return 1;
    }

    // 8. Вывод отчета
    printReportFile(report_filename);

    // 9. Завершение работы
    return 0;
}