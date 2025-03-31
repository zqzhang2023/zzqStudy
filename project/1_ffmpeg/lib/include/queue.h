// Queue.h
#ifndef QUEUE_H
#define QUEUE_H
#include <iostream>
#include <stdexcept>
using namespace std;
// 定义模板类 Queue
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    Node* frontNode;
    Node* rearNode;
    int size;

public:
    Queue();
    ~Queue();
    void push(const T& value);
    void pop();
    bool empty() const;
    T front() const;
};


// 构造函数
template <typename T>
Queue<T>::Queue() : frontNode(nullptr), rearNode(nullptr), size(0) {}

// 析构函数，释放所有节点
template <typename T>
Queue<T>::~Queue() {
    while (!empty()) {
        pop();
    }
}

// 入队操作
template <typename T>
void Queue<T>::push(const T& value) {
    Node* newNode = new Node(value);
    if (rearNode) {
        rearNode->next = newNode;
    } else {
        frontNode = newNode;
    }
    rearNode = newNode;
    size++;
}

// 出队操作
template <typename T>
void Queue<T>::pop() {
    if (empty()) {
        cout<<"队列是空的"<<endl;
        return;
    }
    Node* temp = frontNode;
    frontNode = frontNode->next;
    if (!frontNode) {
        rearNode = nullptr;
    }
    delete temp;
    size--;
}

// 判空
template <typename T>
bool Queue<T>::empty() const {
    return frontNode == nullptr;
}

// 获取队头元素
template <typename T>
T Queue<T>::front() const {
    if (empty()) {
        throw runtime_error("队列是空的，无法获取元素");
    }
    return frontNode->data;
}


#endif
