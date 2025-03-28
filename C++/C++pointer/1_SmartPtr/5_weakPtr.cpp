#include<iostream>
#include <memory>
using namespace std;

template<typename T>
class weakPtr{
private:
    T* _ptr;
public:
    weakPtr():_ptr(nullptr){};
    //拷贝构造
    weakPtr(weakPtr<T>& wp):_ptr(wp._ptr){}
    //share_prt 构造
    weakPtr(shared_ptr<T>& sp):_ptr(sp.get()){}
    //赋值函数
	weakPtr<T>& operator=(weakPtr<T>& wp){
        if(_ptr == wp._ptr){
            return *this;
        }
		_ptr = wp._ptr;
        return *this;
	}
	//通过shared_ptr对象赋值
	weakPtr<T>& operator=(shared_ptr<T>& sp){
		_ptr = sp.get();
        return *this;
	}

    // 添加get方法用于测试
    T* get() const { return _ptr; }
};

int main(){

    // 创建一个shared_ptr并初始化
    shared_ptr<int> sp1(new int(42));
    cout << "sp1.get() = " << sp1.get() << ", *sp1 = " << *sp1 << endl;

    // 测试从shared_ptr构造weakPtr
    weakPtr<int> wp1(sp1);
    std::cout << "wp1.get() = " << wp1.get() << std::endl;

    // 测试拷贝构造
    weakPtr<int> wp2(wp1);
    std::cout << "wp2.get() = " << wp2.get() << std::endl;

    // 测试赋值运算符（weakPtr赋值给weakPtr）
    weakPtr<int> wp3;
    wp3 = wp2;
    std::cout << "wp3.get() = " << wp3.get() << std::endl;

    // 测试从新的shared_ptr赋值
    std::shared_ptr<int> sp2(new int(100));
    wp3 = sp2;
    std::cout << "sp2.get() = " << sp2.get() << ", *sp2 = " << *sp2 << std::endl;
    std::cout << "wp3.get() after sp2 assignment = " << wp3.get() << std::endl;

    // 验证所有weakPtr的指针值
    std::cout << "\nFinal pointers:" << std::endl;
    std::cout << "wp1 (from sp1): " << wp1.get() << std::endl;
    std::cout << "wp2 (copy of wp1): " << wp2.get() << std::endl;
    std::cout << "wp3 (assigned sp2): " << wp3.get() << std::endl;

 

    return 0;
}