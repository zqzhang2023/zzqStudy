#include <iostream>
using namespace std;
 
/*
int Max(int a, int b)
{
    return a>b ? a:b;
}
char Max(char a, char b)
{
    return a>b ? a:b;
}
float Max(float a, float b)
{
    return a>b ? a:b;
}
*/
//template 关键字告诉 C++编译器 我要开始泛型编程了,请你不要随意报错
//T - 参数化数据类型
template <typename T>
T Max(T a, T b){
    return a>b ? a:b;
}
 
int main()
{
    int x = 1;
    int y = 2;
    cout<<"max(1, 2) = "<<Max(x, y)<<endl; //实现参数类型的自动推导
    cout<<"max(1, 2) = "<<Max<int>(x,y)<<endl;//显示类型调用
    float a = 2.0;
    float b = 3.0;
    cout<<"max(2.0, 3.0) = "<<Max(a, b)<<endl;
    
    return 0;
}