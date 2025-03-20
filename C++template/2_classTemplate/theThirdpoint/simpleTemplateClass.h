#ifndef SIMPLE_TEMPLATE_CLASS
#define SIMPLE_TEMPLATE_CLASS

template<typename T>
class A {
public:
    A(T t);
 
    T& getT();
public:
    T t;
};


#endif