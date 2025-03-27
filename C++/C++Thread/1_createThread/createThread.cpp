#include<iostream>
#include<thread>
using namespace std;

void ThreadFunctionA(int A){
    for(int i=0;i<A;i++){
        cout<<"线程 A："<<i<<endl;
    }
}

void ThreadFunctionB(int B){
    for(int i=0;i<B;i++){
        cout<<"线程 B："<<i<<endl;
    }
}

int main(){

    thread ThreadA(ThreadFunctionA,10);
    thread ThreadB(ThreadFunctionB,5);

    ThreadA.join();ThreadB.join();
    
    return 0;
}