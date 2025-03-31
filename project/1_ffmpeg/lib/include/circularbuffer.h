// CircularBuffer.h
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <vector>
#include <stdexcept>

using namespace std;

template <typename T, int Capacity>
class CircularBuffer {
private:
    vector<T> buffer;
    int head;
    int tail;
    int count;

public:
    CircularBuffer();
    bool empty() const;
    bool full() const;
    bool push(const T& item);
    bool pop();
    T front() const;
};


template <typename T, int Capacity>
CircularBuffer<T, Capacity>::CircularBuffer() : buffer(Capacity), head(0), tail(0), count(0) {}

template <typename T, int Capacity>
bool CircularBuffer<T, Capacity>::empty() const {
    return count == 0;
}

template <typename T, int Capacity>
bool CircularBuffer<T, Capacity>::full() const {
    return count == Capacity;
}

template <typename T, int Capacity>
bool CircularBuffer<T, Capacity>::push(const T& item) {
    if (full()) {
        return false; // 缓冲区已满
    }
    buffer[tail] = item;
    tail = (tail + 1) % Capacity;
    count++;
    return true;
}

template <typename T, int Capacity>
bool CircularBuffer<T, Capacity>::pop() {
    if (empty()) {
        return false;
    }
    head = (head + 1) % Capacity;
    count--;
    return true;
}

template <typename T, int Capacity>
T CircularBuffer<T, Capacity>::front() const {
    if (empty()) {
        throw runtime_error("缓冲区为空");
    }
    return buffer[head];
}

#endif