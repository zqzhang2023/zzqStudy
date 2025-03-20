#include"simpleTemplateClass.h"

template<typename T>
A<T>::A(T t){
    this->t=t;
}

template<typename T>
T& A<T>::getT(){
    return this->t;
}

