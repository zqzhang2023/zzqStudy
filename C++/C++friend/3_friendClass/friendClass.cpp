#include<iostream>
#include <sstream>

using namespace std;

// 前向声明
class ComComputerServiceputer;

class Computer { 
public:
    Computer(){
        cpu = "i7";
    };
    string description(){
        stringstream ret; 
        ret << "CPU:" << cpu; 
        return ret.str();
    };
private: 
    string cpu; //CPU 芯片

    friend class ComputerService;

};

// 友元类可以直接访问声明类之中的所有的参数，包括私有参数
class ComputerService { 
public:
    void upgrade(Computer* computer){
        computer->cpu = "i9";
    }; 
    void clean(Computer* computer){
        std::cout << "正在对电脑执行清理[CPU:"
        << computer->cpu << "]..."
        << std::endl;
    }; //计算机清理 
    void kill(Computer* computer){
        std::cout << "正在对电脑执行杀毒[CPU:"
        << computer->cpu << "]..."
        << std::endl;
    }; //杀毒
};


int main(){

    Computer shanxing; 
    ComputerService service;
 
    std::cout << shanxing.description() << std::endl;
    service.upgrade(&shanxing); 
    service.clean(&shanxing); 
    service.kill(&shanxing);
    std::cout << shanxing.description() << std::endl;


    return 0;
}