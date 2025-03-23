#include <iostream>
#include <thread>
#include <condition_variable>
using namespace std;

mutex mtx;  // 互斥量
condition_variable cv;  // 条件变量
bool isReady = false;  // 条件

void thread_function()
{
    unique_lock<mutex> lock(mtx);
    while (!isReady) 
    {
        cv.wait(lock);  // 等待条件满足
    }
    cout << "Thread is notified" << endl;
}

int main()
{
    thread t(thread_function);

    // 模拟一段耗时操作
    this_thread::sleep_for(chrono::seconds(2));

    {
        lock_guard<mutex> lock(mtx);
        isReady = true;  // 设置条件为true
    }
    cv.notify_one();  // 通知等待的线程

    t.join();

    return 0;
}
