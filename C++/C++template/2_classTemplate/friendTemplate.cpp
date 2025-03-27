#include<iostream>
using namespace std;

// 前置声明模板类和模板函数
template<typename T> class A;
template<typename T> A<T> addA(const A<T>& a, const A<T>& b);

template<typename T>
class A {
public:

    A();

    A(T t);
 
    T& getT();

    friend A<T> addA<T>(const A<T>& a, const A<T>& b); 

    void print();
public:
    T t;
};

template<typename T>
A<T>::A(){}


template<typename T>
A<T>::A(T t){
    this->t=t;
}

template<typename T>
T& A<T>::getT(){
    return this->t;
}


template<typename T>
void A<T>::print(){
     cout<<this->t<<endl;
 }

template<typename T>
A<T> addA(const A<T> &a, const A<T> &b){
    A<T> tmp;
    cout<<"call add A()..."<<endl;
    tmp.t=a.t+b.t;
    return tmp;
}


int main(){
    A<int> a(666),b(888);
    //cout<<a.getT()<<endl;
    A<int> tmp1=addA<int>(a,b);
    tmp1.print();


     return 0;
}

