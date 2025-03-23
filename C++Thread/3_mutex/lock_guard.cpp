#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex mtx;  // 互斥量

void thread_function()
{
    lock_guard<mutex> lock(mtx);  // 加锁互斥量
    cout << "Thread running" << endl;
    // 执行需要加锁保护的代码
}  // lock_guard对象的析构函数自动解锁互斥量

int main()
{
    thread t1(thread_function);
    t1.join();
    cout << "Main thread exits!" << endl;
    return 0;
}
