#include<iostream>
using namespace std;

template<typename T>
class uniquePtr{
private:
    T* _ptr;
public:
    uniquePtr(T* ptr):_ptr(ptr){};
    ~uniquePtr(){
        delete _ptr;
    }

    uniquePtr(const uniquePtr<T>& up) = delete;
    uniquePtr<T> & operator=(const uniquePtr<T>& up) = delete;

    T& operator*(){
        return *_ptr;
    }

    T* operator->(){
        return _ptr;
    }

};

struct TestStruct {
    int val;
    TestStruct(int v) : val(v) {}
};



int main(){

    {// 基础功能测试
        uniquePtr<int> up1(new int(42));
        cout << *up1 << endl; // 输出: 42

        uniquePtr<TestStruct> up2(new TestStruct{99});
        cout << up2->val << endl; // 输出: 99
    }

    //{
    //    // 以下代码取消注释会编译错误（测试拷贝禁止）
    //    uniquePtr<int> up3(new int(42));
    //    uniquePtr<int> up4(up3);
    //    uniquePtr<int> up5 = up3;          // 错误：拷贝构造被删除
    //    uniquePtr<int> up6; up6 = up4;     // 错误：拷贝赋值被删除
    //}


    return 0;
}