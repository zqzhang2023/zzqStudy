#include<iostream>
#include<thread>
#include<chrono>
using namespace std;

void my_thread(){
    cout<<"Thread ID"<<this_thread::get_id()<<" start! "<<endl;
    for (int i = 0; i < 5; i++){
        cout << "Thread " << this_thread::get_id() << " running: " << i << endl;
        this_thread::yield();	// 让出当前线程的时间片
        this_thread::sleep_for(chrono::milliseconds(200));  // 线程休眠200毫秒
    }
    cout << "Thread " << this_thread::get_id() << " end！" << endl;
}

int main(){

    cout << "Main thread id: " << this_thread::get_id() << endl;
	
    thread t1(my_thread);
	thread t2(my_thread);
	
	t1.join();
	t2.join();

    return 0;
}