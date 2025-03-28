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


template<typename T, typename Del = Delete<T>>
class sharedPtr{
private:
    T* _ptr;
    long int* _pcount;   //引用计数
public:
    sharedPtr(T* ptr):_ptr(ptr),_pcount(new long int(1)){};
    //拷贝构造
    sharedPtr(sharedPtr<T,Del> & sp):_ptr(sp._ptr),_pcount(sp._pcount){
        ++(*_pcount); //计数+1
    }
    //赋值函数
    sharedPtr<T,Del>& operator=(sharedPtr<T,Del> & sp){
        if(_ptr == sp._ptr){
            return *this;
        }
        //this的引用计数-1，并判断是否需要释放资源
        (*_pcount)--; //
        if((*_pcount)==0){
            Del del;
            del(_ptr); _ptr = nullptr;
            delete _pcount; _pcount = nullptr;
        }
        //赋值
        _ptr = sp._ptr;
        _pcount = sp._pcount;
        (*_pcount)++;
        return *this;
    }

    //指针获取函数
    T* get(){
        return _ptr;
    }

    //引用计数获取函数
    long int use_count(){
        return *_pcount;
    }

    T& operator*(){
        return *_ptr;
    }

    T* operator->(){
        return _ptr;
    }

    bool unique(){
		return *_pcount == 1;
	}

    //析构函数
    ~sharedPtr(){
        (*_pcount)--; //
        if((*_pcount)==0){
            Del del;
            del(_ptr); _ptr = nullptr;
            delete _pcount; _pcount = nullptr;
        }
    }


};

int main(){

    // 测试基础类型和引用计数
    sharedPtr<int> p1(new int(42));
    cout << "p1 use count: " << p1.use_count() << endl;  // 1

    {
        sharedPtr<int> p2 = p1;
        cout << "p1 use count after copy: " << p1.use_count() << endl;  // 2
        cout << "p2 use count: " << p2.use_count() << endl;             // 2
    }
    cout << "p1 use count after p2销毁: " << p1.use_count() << endl;    // 1

    // 测试赋值运算符
    sharedPtr<int> p3(new int(100));
    p1 = p3;
    cout << "p1 use count after assignment: " << p1.use_count() << endl; // 2
    cout << "p3 use count: " << p3.use_count() << endl;                  // 2

    // 测试自赋值
    p1 = p1;
    cout << "p1 use count after self-assignment: " << p1.use_count() << endl; // 2

    // 测试数组类型
    sharedPtr<int, DeleteArray<int>> arr(new int[5]);
    // 此处arr析构时会调用delete[]

    // 测试malloc/free
    sharedPtr<int, Free<int>> malloc_ptr(static_cast<int*>(malloc(sizeof(int))));
    // 此处malloc_ptr析构时会调用free

    return 0;
}