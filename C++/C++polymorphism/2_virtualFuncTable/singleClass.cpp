#include<iostream>
using namespace std;

class Father{

public: //为了便于测试，特别该用 public
    int x = 100;
    int y = 200;
    static int z;

public:
    virtual void func1(){ cout << "Father::func1" << endl; }
    virtual void func2(){ cout << "Father::func2" << endl; }
    virtual void func3(){ cout << "Father::func3" << endl; }
    void func4() { cout << "非虚函数：Father::func4" << endl; }

};
//静态数据初始化
int Father::z = 1;
typedef void (*func_t)(void);
int main(){
    Father father;

    // 安全地获取对象地址
    cout << "对象地址：" << static_cast<void*>(&father) << endl;

    // 安全地获取虚函数表指针
    void** vptr = *reinterpret_cast<void***>(&father);
    cout << "虚函数表指针 vptr：" << vptr << endl;

    // 调用虚函数表中的函数
    cout << "调用第 1 个虚函数: ";
    reinterpret_cast<func_t>(vptr[0])();
    cout << "调用第 2 个虚函数：";
    reinterpret_cast<func_t>(vptr[1])();
    cout << "调用第 3 个虚函数: ";
    reinterpret_cast<func_t>(vptr[2])();

    // 安全地访问数据成员
    cout << "第 1 个数据成员的地址: " << &father.x << endl;
    cout << "第 1 个数据成员的值：" << father.x << endl;

    cout << "第 2 个数据成员的地址: " << &father.y << endl;
    cout << "第 2 个数据成员的值：" << father.y << endl;

    cout << "sizeof(father)==" << sizeof(father) << endl;

    Father father2;
    cout << "father 的虚函数表：" << *reinterpret_cast<void***>(&father) << endl;
    cout << "father2 的虚函数表：" << *reinterpret_cast<void***>(&father2) << endl;

    return 0;

}