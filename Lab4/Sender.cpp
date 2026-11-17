#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "common.h"

using namespace std;

sem_t* openSem(const string& name) {
    sem_t* s = sem_open(name.c_str(), 0);
    if (s == SEM_FAILED) { perror("sem_open"); exit(1); }
    return s;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { cout << "Использование: ./Sender <имя файла>\n"; return 1; }
    string file = argv[1];

    int fd = open(file.c_str(), O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    sem_t* mutex = openSem(semName(file, "mtx"));
    sem_t* freeSlots = openSem(semName(file, "emp"));
    sem_t* fullSlots = openSem(semName(file, "ful"));
    sem_t* ready = openSem(semName(file, "rdy"));

    sem_post(ready);

    while (true) {
        cout << "1 - отправить сообщение, 0 - завершить работу\n> ";
        int cmd;
        if (!(cin >> cmd)) break;
        if (cmd == 0) break;
        if (cmd != 1) continue;

        cout << "Сообщение (меньше " << MESSAGE_SIZE << " символов): ";
        string text;
        cin >> ws;
        getline(cin, text);
        if (text.size() >= MESSAGE_SIZE) {
            cout << "Слишком длинное сообщение\n";
            continue;
        }

        char msg[MESSAGE_SIZE] = {0};
        strcpy(msg, text.c_str());

        sem_wait(freeSlots);
        sem_wait(mutex);

        QueueHeader header;
        readHeader(fd, header);
        pwrite(fd, msg, MESSAGE_SIZE, recordOffset(header.tail));
        header.tail = (header.tail + 1) % header.capacity;
        writeHeader(fd, header);

        sem_post(mutex);
        sem_post(fullSlots);
    }

    close(fd);
    sem_close(mutex); sem_close(freeSlots); sem_close(fullSlots); sem_close(ready);
    return 0;
}