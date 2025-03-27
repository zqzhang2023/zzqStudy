#include <iostream>
using namespace std;

int value;   //定义全局变量value

int main() 
{
    int value;  //定义局部变量value
    value = 100;
    ::value = 1000;
    cout << "local value : " << value << endl;
    cout << "global value : " << ::value << endl;
    return 0;
}