#include<iostream>
#include <sstream>

using namespace std;

// 前向声明
class Computer;

class ComputerService { 
public:
    void upgrade(Computer* computer);
};


class Computer { 
public:
    Computer(){
        cpu = "i7";
    };
    // 使用全局函数作为友元函数
    friend void upgrade(Computer* computer);

    friend void ComputerService::upgrade(Computer* computer);

    string description(){
        stringstream ret; 
        ret << "CPU:" << cpu; 
        return ret.str();
    };
private: 
    string cpu; //CPU 芯片
};

// ComputerService成员函数的实现（此时Computer已定义，可访问私有成员）
void ComputerService::upgrade(Computer* computer) {
    computer->cpu = "i9";
}

//直接访问对象的私有数据成员！！！! ! ! ! !
void upgrade(Computer* computer) {
    computer->cpu = "i9"; //直接访问对象的私有数据成员！！！
}

int main(){

    Computer shanxing; 
    ComputerService service;
    std::cout << shanxing.description() << std::endl;
    service.upgrade(&shanxing);
    std::cout << shanxing.description() << std::endl;


    return 0;
}