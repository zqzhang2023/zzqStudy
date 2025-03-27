#include<iostream>
using namespace std;

/*
template<typename T>
class A {
public:
    A(T t){
        this->t=t;
    }
 
    T& getT(){
        return t;
    }
public:
    T t;
};
*/

template<typename T>
class A {
public:
    A(T t);
 
    T& getT();
public:
    T t;
};

template<typename T>
A<T>::A(T t){
    this->t=t;
}

template<typename T>
T& A<T>::getT(){
    return this->t;
}

//这里还是需要指明参数的 A<int> &a
void printA(A<int> &a){
     cout<<a.getT()<<endl;
}

int main(){
     //1.模板类定义类对象，必须显示指定类型
     //2.模板种如果使用了构造函数，则遵守以前的类的构造函数的调用规则
     A<int> a(666);

     cout<<a.getT()<<endl;
     //模板类做为函数参数
     printA(a);

     return 0;
}