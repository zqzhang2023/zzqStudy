#include <iostream>
#include <string>

// 定义一个模板类
template <typename T>
class MyClass {
private:
    static T staticValue; // 静态成员变量
    T value;              // 普通成员变量

public:
    // 构造函数
    MyClass(T val) : value(val) {
        staticValue = val; // 初始化静态成员变量
    }

    // 静态成员函数：打印静态成员变量
    static void printStaticValue() {
        std::cout << "Static Value: " << staticValue << std::endl;
    }

    // 静态成员函数：设置静态成员变量
    static void setStaticValue(const T& newVal) {
        staticValue = newVal;
    }

    // 普通成员函数
    void printValue() const {
        std::cout << "Value: " << value << std::endl;
    }
};

// 显式模板实例化 ！！！！！
template <typename T>
T MyClass<T>::staticValue = T(); // 直接初始化静态成员变量

int main() {
    // 创建两个对象
    MyClass<int> obj1(10);
    MyClass<int> obj2(20);

    // 打印静态成员变量的初始值
    obj1.printStaticValue(); // 输出：Static Value: 0
    obj2.printStaticValue(); // 输出：Static Value: 0

    // 使用静态成员函数修改静态成员变量
    MyClass<int>::setStaticValue(100);

    // 再次打印静态成员变量的值
    obj1.printStaticValue(); // 输出：Static Value: 100
    obj2.printStaticValue(); // 输出：Static Value: 100

    // 调用普通成员函数
    obj1.printValue();       // 输出：Value: 10
    obj2.printValue();       // 输出：Value: 20

    // 创建一个不同类型的对象
    MyClass<std::string> obj3("Hello");

    // 打印静态成员变量的初始值
    obj3.printStaticValue(); // 输出：Static Value: (空字符串)

    // 修改静态成员变量
    MyClass<std::string>::setStaticValue("World");

    // 再次打印静态成员变量的值
    obj3.printStaticValue(); // 输出：Static Value: World

    return 0;
}