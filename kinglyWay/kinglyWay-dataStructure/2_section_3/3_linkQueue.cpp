#include<iostream>
using namespace std;

enum operation_code {success,overflow,underflow};

template <typename T>
struct Node{
    T data;
    Node<T>* next;
};

template <typename T>
class Queue{
private:
    Node<T>* front;
    Node<T>* rear;
    int size;
public:
    Queue(){
        front = new Node<T>();
        front->next = nullptr;
        rear = front;
        size = 0;
    }

    //判空
    bool isEmpty(){
        // return rear == front;
        return size == 0;
    }

    //入队
    operation_code push(T x){
        Node<T> * newNode = new Node<T>();
        newNode->data = x;
        newNode->next = nullptr;
        rear->next = newNode;
        rear = newNode;
        size++;
        return success;
    }   

    //出队
    operation_code pop(T &x){
        if (isEmpty()){
            return underflow;
        }
        Node<T> *tmp = front->next;
        x = tmp->data;
        front->next = tmp->next;
        if (rear == tmp){
            rear = front;
        }
        delete tmp;
        size--;
        return success;      
    }

    ~Queue(){
        while (front != nullptr) {
            Node<T>* tmp = front;
            front = front->next;
            delete tmp;
        }
    }
};




int main(){
    Queue<int> q;
    int val;

    // 测试空队列出队
    cout << "空队列出队结果: " << q.pop(val) << " (应返回2 underflow)" << endl;

    // 入队测试
    q.push(10);
    q.push(20);
    q.push(30);

    // 出队测试
    q.pop(val);
    cout << "第一次出队元素: " << val << " (应输出10)" << endl;
    q.pop(val);
    cout << "第二次出队元素: " << val << " (应输出20)" << endl;

    // 再次入队
    q.push(40);
    q.pop(val);
    cout << "第三次出队元素: " << val << " (应输出30)" << endl;
    q.pop(val);
    cout << "第四次出队元素: " << val << " (应输出40)" << endl;

    // 测试空队列
    cout << "空队列出队结果: " << q.pop(val) << " (应返回2 underflow)" << endl;

    return 0;
}