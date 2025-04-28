#include<iostream>
using namespace std;

int main(){

    int a = 0;
    int b = 1;
    // 常量指针 常量的指针，指向常量的指针，所以指针指向的值是不变的, 但是可以改变其所指的内容
    const int* ptr = &a;
    // 可以改变指针所指向的地址
    cout<<*ptr<<endl;
    ptr = &b;

    // 但是不可所指向的地址的值
    // *ptr = 3; //这会报错
    cout<<*ptr<<endl;
    b = 3; // 但是可以通过变量来改变
    cout<<*ptr<<endl;


    // 指针常量 是一个常量，这个指针是常量代表着不可以改变这个指针所指的地址（因为指针的值本身就是地址）
    int* const ptr_1 = &a;
    cout<<*ptr_1<<endl;
    // 可以改变这个地址里面的值
    *ptr_1 = 333;
    cout<<*ptr_1<<endl;
    // 不可以改变指针所指的内容
    // ptr_1 = &b; //这会报错


    return 0;
}