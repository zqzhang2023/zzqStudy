#include<iostream>
using namespace std;

int main(){
    
    // void指针
    void * pc;
    // int型
    int i = 123;
    cout<<i<<endl;
    // char型
    char c = 'a';
    cout<<c<<endl;

    // int * 赋值给 void * 使用的时候需要进行强制转化
    pc = &i;
    cout<< *(int *)pc <<endl;

    // int * 赋值给 void * 使用的时候需要进行强制转化
    pc = &c;
    cout<< *(char *)pc <<endl;


    return 0;
}