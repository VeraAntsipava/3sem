#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include "common.h"

using namespace std;

string getFileName() {
    while (true) {
        cout << "1 - ввести имя файла с консоли\n"
             << "2 - получить имя файла из переменной среды " << ENV_NAME << "\n> ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        if (choice == 1) {
            string name;
            cout << "Имя бинарного файла: ";
            cin >> name;
            return name;
        }
        if (choice == 2) {
            const char* value = getenv(ENV_NAME);
            if (value == nullptr || *value == '\0') {
                cout << "Переменная среды " << ENV_NAME << " не найдена.\n";
                continue;
            }
            return value;
        }
    }
}

void launchSender(const string& file, int index) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string script = "/tmp/sender_" + to_string(getpid()) + "_" + to_string(index) + ".command";
    FILE* f = fopen(script.c_str(), "w");
    if (!f) { perror("fopen"); exit(1); }
    fprintf(f, "#!/bin/bash\ncd \"%s\"\n./Sender \"%s\"\n", cwd, file.c_str());
    fclose(f);
    chmod(script.c_str(), 0755);
    if (fork() == 0) {
        execl("/usr/bin/open", "open", "-a", "Terminal", script.c_str(), nullptr);
        _exit(1);
    }
}

sem_t* createSem(const string& name, unsigned value) {
    sem_unlink(name.c_str());
    sem_t* s = sem_open(name.c_str(), O_CREAT | O_EXCL, 0644, value);
    if (s == SEM_FAILED) { perror("sem_open"); exit(1); }
    return s;
}

int main() {
    string file = getFileName();

    int capacity;
    cout << "Количество записей в файле: ";
    cin >> capacity;
    if (capacity <= 0) { cout << "Некорректное количество записей\n"; return 1; }

    int fd = open(file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return 1; }
    QueueHeader header = {capacity, 0, 0};
    writeHeader(fd, header);
    char empty[MESSAGE_SIZE] = {0};
    for (int i = 0; i < capacity; i++)
        pwrite(fd, empty, MESSAGE_SIZE, recordOffset(i));

    int senders;
    cout << "Количество процессов Sender: ";
    cin >> senders;

    sem_t* mutex = createSem(semName(file, "mtx"), 1);
    sem_t* freeSlots = createSem(semName(file, "emp"), capacity);
    sem_t* fullSlots = createSem(semName(file, "ful"), 0);
    sem_t* ready = createSem(semName(file, "rdy"), 0);

    for (int i = 0; i < senders; i++) launchSender(file, i);

    cout << "Ожидание готовности Sender'ов...\n";
    for (int i = 0; i < senders; i++) sem_wait(ready);
    cout << "Все Sender'ы готовы.\n";

    while (true) {
        cout << "1 - прочитать сообщение, 0 - завершить работу\n> ";
        int cmd;
        if (!(cin >> cmd)) break;
        if (cmd == 0) break;
        if (cmd != 1) continue;

        sem_wait(fullSlots);
        sem_wait(mutex);

        readHeader(fd, header);
        char msg[MESSAGE_SIZE];
        pread(fd, msg, MESSAGE_SIZE, recordOffset(header.head));
        header.head = (header.head + 1) % header.capacity;
        writeHeader(fd, header);

        sem_post(mutex);
        sem_post(freeSlots);

        cout << "Сообщение: " << msg << "\n";
    }

    close(fd);
    for (const char* kind : {"mtx", "emp", "ful", "rdy"}) sem_unlink(semName(file, kind).c_str());
    sem_close(mutex); sem_close(freeSlots); sem_close(fullSlots); sem_close(ready);
    return 0;
}