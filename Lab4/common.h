#pragma once
#include <cstdio>
#include <string>
#include <unistd.h>

const int MESSAGE_SIZE = 20;
const char* const ENV_NAME = "Antsipava";

struct QueueHeader {
    int capacity;
    int head;
    int tail;
};

inline unsigned hashName(const std::string& s) {
    unsigned h = 2166136261u;
    for (unsigned char c : s) { h ^= c; h *= 16777619u; }
    return h;
}

inline std::string semName(const std::string& file, const char* kind) {
    char buf[32];
    snprintf(buf, sizeof(buf), "/l4_%s_%08x", kind, hashName(file));
    return buf;
}

inline off_t recordOffset(int index) {
    return sizeof(QueueHeader) + (off_t)index * MESSAGE_SIZE;
}

inline void readHeader(int fd, QueueHeader& h)        { pread(fd, &h, sizeof(h), 0); }
inline void writeHeader(int fd, const QueueHeader& h) { pwrite(fd, &h, sizeof(h), 0); }