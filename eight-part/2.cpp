#include<iostream>
using namespace std;

int add(int a,int b){
    return a + b;
}

// 指针函数，返回指针的函数
int* getPointer(){
    int x = 0;
    return &x;
}

int main(){
    // 函数指针 就是指向函数的指针
    int (*ptr)(int,int) = &add;
    // 两种调用方法：
    // 1.隐式调用
    int result = ptr(3,4);
    cout<<result<<endl;
    // 2.显式调用
    result = (*ptr)(5,6);
    cout<<result<<endl;

    return 0;
}