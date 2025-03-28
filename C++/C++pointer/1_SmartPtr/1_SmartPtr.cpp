#include<iostream>
using namespace std;

template<typename T>
struct Delete {
    void operator()(T * ptr){
        delete ptr;
    }
};

template<typename T>
struct DeleteArray {
    void operator()(T * ptr){
        delete[] ptr;
    }
};

template<typename T>
struct Free {
    void operator()(T * ptr){
        free(ptr);
    }
};


template<typename T,typename Del = Delete<T>>
class SmartPtr{
private:
    T* _ptr;
public:
    SmartPtr(T* ptr=nullptr):_ptr(ptr){};
    ~SmartPtr(){
        Del del;
        del(_ptr);
    }

    //基础访问功能
    T* get() const noexcept { return _ptr; }

    T& operator*()
	{
		return *_ptr;
	}
    
    //这里解释一下 当用户写下类似`ap->member`时，
    //实际上会被转换成`(ap.operator->())->member`
	T* operator->()
	{
		return _ptr;
	}

};


// 自定义一个简单的结构体用于测试operator->
struct Point {
    int x;
    int y;
};


int main(){
    // 测试默认删除器（Delete<T>）
    {
        SmartPtr<int> ptr1(new int(10));
        cout << "Default Deleter Test: " << *ptr1 << endl;  // 输出10
        *ptr1 = 20;
        cout << "Modified Value: " << *ptr1 << endl;         // 输出20
    } // 此处自动调用delete

    // 测试数组删除器（DeleteArray<T>）
    {
        SmartPtr<int, DeleteArray<int>> ptr2(new int[3]{1, 2, 3});
        cout << "Array Test: ";
        cout << ptr2.get()[0] << ", " << ptr2.get()[1] << ", " << ptr2.get()[2] << endl; // 输出1,2,3
    } // 此处自动调用delete[]

    // 测试C风格内存释放（Free<T>）
    {
        int* raw_ptr = static_cast<int*>(malloc(sizeof(int)));
        *raw_ptr = 30;
        SmartPtr<int, Free<int>> ptr3(raw_ptr);
        cout << "Free Test: " << *ptr3 << endl; // 输出30
    } // 此处自动调用free

    // 测试结构体与operator->
    {
        SmartPtr<Point> ptr4(new Point);
        ptr4->x = 100;    // 通过operator->修改成员
        ptr4->y = 200;
        cout << "Struct Test: " << ptr4->x << ", " << ptr4->y << endl; // 输出100,200
    } // 此处自动调用delete

    { //对结构体使用operator*
        SmartPtr<Point> ptr6(new Point{300, 400});
        cout << "Struct Dereference Test:" << endl;
        
        // 通过operator*访问结构体
        (*ptr6).x = 500;  // 等价于ptr6->x = 500
        (*ptr6).y = 600;
        
        // 混合使用两种访问方式
        cout << "Modified values: " 
             << (*ptr6).x << ", "    // 500
             << ptr6->y << endl;      // 600
    }

    return 0;
}