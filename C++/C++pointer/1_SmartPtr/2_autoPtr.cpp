#include<iostream>
using namespace std;

template<typename T>
class autoPtr{
private:
    T * _ptr;
public:
    //构造函数
    autoPtr(T* ptr = nullptr):_ptr(ptr){};
    //拷贝构造
    autoPtr(autoPtr<T> & ap):_ptr(ap._ptr){
        ap._ptr = nullptr;
    }
    //赋值构造
    autoPtr<T>& operator=(autoPtr<T> & ap){
        if (_ptr != ap._ptr){  //自赋值
            delete this->_ptr;
            this->_ptr = ap._ptr;
            ap._ptr = nullptr;
        }
        return *this;
    }

    T& operator*(){
        return *_ptr;
    }

    T* operator->(){
        return _ptr;
    }

    // 析构函数
    ~autoPtr() {
        delete _ptr;
    }

    // 辅助函数：检查指针是否为空
    bool isNull() const {
        return _ptr == nullptr;
    }
};

// 测试类
struct Test {
    int value;
    Test(int v) : value(v) {}
    ~Test() {
        std::cout << "Test destroyed: " << value << std::endl;
    }
};

int main(){
    // 测试1: 基础构造和析构
    {
        std::cout << "--- Test 1: Basic ---" << std::endl;
        autoPtr<Test> ap1(new Test(1));
    } // 此处应输出"Test destroyed: 1"

    // 测试2: 拷贝构造所有权转移
    {
        std::cout << "\n--- Test 2: Copy Constructor ---" << std::endl;
        autoPtr<Test> ap1(new Test(2));
        autoPtr<Test> ap2(ap1);  // 所有权转移
        
        std::cout << "ap1.isNull(): " << ap1.isNull() << std::endl;  // 应输出1
        std::cout << "ap2->value: " << ap2->value << std::endl;     // 应输出2
    } // 此处应输出"Test destroyed: 2"

    // 测试3: 赋值运算符所有权转移
    {
        std::cout << "\n--- Test 3: Assignment Operator ---" << std::endl;
        autoPtr<Test> ap3(new Test(3));
        autoPtr<Test> ap4;
        ap4 = ap3;  // 所有权转移
        
        std::cout << "ap3.isNull(): " << ap3.isNull() << std::endl; // 应输出1
        std::cout << "ap4->value: " << ap4->value << std::endl;     // 应输出3
    } // 此处应输出"Test destroyed: 3"

    // 测试4: 操作符功能
    {
        std::cout << "\n--- Test 4: Operators ---" << std::endl;
        autoPtr<Test> ap5(new Test(5));
        (*ap5).value = 6;
        std::cout << "Modified value via *: " << ap5->value << std::endl;  // 应输出6
        
        ap5->value = 7;
        std::cout << "Modified value via ->: " << (*ap5).value << std::endl; // 应输出7
    } // 此处应输出"Test destroyed: 7"

    // 测试5: 自赋值
    {
        std::cout << "\n--- Test 5: Self-Assignment ---" << std::endl;
        autoPtr<Test> ap6(new Test(6));
        ap6 = ap6;  // 自赋值应安全处理
        
        std::cout << "ap6->value: " << ap6->value << std::endl;  // 应输出6
    } // 此处应输出"Test destroyed: 6"

    return 0;
}