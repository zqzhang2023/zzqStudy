#include<iostream>
#include<thread>
#include<mutex>
using namespace std;

mutex mtx;
int num = 0;


void thread_function(int &n){
    for(int i=0;i<100;i++){
        mtx.lock();
        n++;
        mtx.unlock();
    }
}

int main(){

    thread myThread[500];

    for(thread &a:myThread){
        a = thread(thread_function,ref(num));
        a.join();
    }
    cout << "num = " << num << endl;
	cout << "Main thread exits!" << endl;

    return 0;
}