#include<iostream>
using namespace std;

#define MaxSize 100
enum operation_code {success,overflow,underflow};

template <typename T>
class Queue{
private:
    T data[MaxSize];
    int size;
    int front;
    int rear;
public:
    //构造初始化
    Queue(){
        front = 0;
        rear = 0;
        size = 0;
    }
    //判空
    bool isEmpty(){
        return size==0;
    }
    //判满
    bool isFull(){
        return size==MaxSize;
    }
    //入队
    operation_code push(T x){
        if (isFull()){
            return overflow;
        }
        data[rear]= x;
        rear = (rear + 1)%MaxSize;
        size++;
        return success;
    }
    //出队
    operation_code pop(T &x){
        if (isEmpty()){
            return underflow;
        }
        x = data[front];
        front = (front + 1)%MaxSize;
        size--;
        return success;
    }
};

int main() {
    Queue<int> q;
    int x;

    // 填满队列
    for (int i = 0; i < 100; ++i) {
        if (q.push(i) != success) {
            cout << "队列已满，无法入队元素 " << i << endl;
        }
    }

    // 测试满队列入队
    operation_code result = q.push(100);
    cout << "满队列入队结果: " << result << " (1 表示overflow)" << endl;

    // 清空队列
    while (q.pop(x) == success) {
        cout << x << " ";
    }
    cout << "\n队列已清空" << endl;

    // 测试空队列出队
    result = q.pop(x);
    cout << "空队列出队结果: " << result << " (2 表示underflow)" << endl;

    // 正常入队和出队测试
    q.push(10);
    q.push(20);
    q.pop(x);
    cout << "出队元素: " << x << endl;  // 应输出10
    q.pop(x);
    cout << "出队元素: " << x << endl;  // 应输出20

    return 0;
}