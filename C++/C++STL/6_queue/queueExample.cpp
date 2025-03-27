#include<iostream>
#include<queue>
using namespace std;

void traverseQueue(queue<int> q) { // 传值创建副本
    while (!q.empty()) {
        cout << q.front() << " ";
        q.pop(); // 弹出副本元素，不影响原队列
    }
    cout<<endl;
}

void traverse(std::queue<int>& q) {
    int size = q.size();
    for (int i = 0; i < size; ++i) {
        int element = q.front();  // 获取队首元素
        q.pop();                   // 弹出元素
        std::cout << element << " "; // 处理元素（此处为打印）
        q.push(element);           // 重新插入队尾
    }
    cout<<endl;
}

int main(){
    queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    q.push(4);

    //遍历 用一个tempQueue先存储一下，然后再push进取
    queue<int> tempQueue;
    while (!q.empty()) {
        int frontElement = q.front();
        cout << frontElement << " ";
        tempQueue.push(frontElement);
        q.pop();
    }
    cout << endl;
    // 将元素再压回原队列
    while (!tempQueue.empty()) {
        q.push(tempQueue.front());
        tempQueue.pop();
    }

    traverseQueue(q);

    traverse(q);



    return 0;
}