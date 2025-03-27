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


class Mother {
public:
    virtual void handle1() { cout << "Mother::handle1" << endl; }
    virtual void handle2() { cout << "Mother::handle2" << endl; }
    virtual void handle3() { cout << "Mother::handle3" << endl; }
public: //为了便于测试，使用 public 权限
    int m = 400;
    int n = 500;
};

class Son : public Father, public Mother {
public:
    void func1() { cout << "Son::func1" << endl; }
    virtual void func5() { cout << "Son::func5" << endl; }
    virtual void handle1() { cout << "Son::handle1" << endl; }
};
 
typedef void (*func_t)(void);

int main() {
    Son son;

    // 输出对象地址
    cout << "son 对象地址：" << static_cast<void*>(&son) << endl;

    // 获取 Father 的虚表指针
    Father* fatherPtr = &son;
    void** vptr_father = *reinterpret_cast<void***>(fatherPtr);
    cout << "Father 的虚表指针：" << vptr_father << endl;

    // 调用 Father 虚表中的函数（前 4 个：func1, func2, func3, func5）
    for (size_t i = 0; i < 4; ++i) {
        if (vptr_father[i]) {
            cout << "Father 虚表第 " << i+1 << " 个函数: ";
            reinterpret_cast<func_t>(vptr_father[i])();
        } else break;
    }

    // 输出 Father 数据成员
    cout << "Father::x = " << son.x << " at " << &son.x << endl;
    cout << "Father::y = " << son.y << " at " << &son.y << endl;

    // 获取 Mother 的虚表指针
    Mother* motherPtr = &son;
    void** vptr_mother = *reinterpret_cast<void***>(motherPtr);
    cout << "Mother 的虚表指针：" << vptr_mother << endl;

    // 调用 Mother 虚表中的函数（3 个）
    for (size_t i = 0; i < 3; ++i) {
        if (vptr_mother[i]) {
            cout << "Mother 虚表第 " << i+1 << " 个函数: ";
            reinterpret_cast<func_t>(vptr_mother[i])();
        } else break;
    }

    // 输出 Mother 数据成员
    cout << "Mother::m = " << son.m << " at " << &son.m << endl;
    cout << "Mother::n = " << son.n << " at " << &son.n << endl;

    return 0;
}
