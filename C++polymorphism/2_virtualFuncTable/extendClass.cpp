#include <iostream>
using namespace std;
class Father {
public:
    virtual void func1() { cout << "Father::func1" << endl; }
    virtual void func2() { cout << "Father::func2" << endl; }
    virtual void func3() { cout << "Father::func3" << endl; }
    void func4() { cout << "非虚函数：Father::func4" << endl; }
public: //为了便于测试，特别该用 public
    int x = 100;
    int y = 200;
};
 
class Son : public Father {
public:
    void func1() { cout << "Son::func1" << endl; }
    virtual void func5() { cout << "Son::func5" << endl; }
};
 
typedef void (*func_t)(void);

int main(void) {
    Father father;
    Son son;

    // 安全地获取对象地址
    cout << "father 对象地址：" << static_cast<void*>(&father) << endl;

    // 安全地获取虚函数表指针
    void** vptr_father = *reinterpret_cast<void***>(&father);
    cout << "father 虚函数表指针 vptr：" << vptr_father << endl;


    // 安全地获取对象地址
    cout << "son 对象地址：" << static_cast<void*>(&son) << endl;

    // 安全地获取虚函数表指针
    void** vptr = *reinterpret_cast<void***>(&son);
    cout << "son 虚函数表指针 vptr：" << vptr << endl;

    // 调用虚函数表中的函数
    for (size_t i = 0; i < 4; i++) {
        if (vptr[i] != nullptr) {
            cout << "调用第" << i + 1 << "个虚函数：";
            reinterpret_cast<func_t>(vptr[i])();
        } else {
            break; // 避免访问无效的函数指针
        }
    }

    // 安全地访问数据成员
    cout << "第 1 个数据成员的地址: " << &son.x << endl;
    cout << "第 1 个数据成员的值：" << son.x << endl;

    cout << "第 2 个数据成员的地址: " << &son.y << endl;
    cout << "第 2 个数据成员的值：" << son.y << endl;

    cout << "sizeof(son)==" << sizeof(son) << endl;

    return 0;
}