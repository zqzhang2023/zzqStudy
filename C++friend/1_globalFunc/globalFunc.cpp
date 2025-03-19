#include<iostream>
#include <sstream>

using namespace std;


class Computer { 
public:
    Computer(){
        cpu = "i7";
    };
    // 使用全局函数作为友元函数
    friend void upgrade(Computer* computer);

    string description(){
        stringstream ret; 
        ret << "CPU:" << cpu; 
        return ret.str();
    };
private: 
    string cpu; //CPU芯片
};

//直接访问对象的私有数据成员！！！! ! ! ! !
void upgrade(Computer* computer) {
    computer->cpu = "i9"; //直接访问对象的私有数据成员！！！
}

int main(){

    Computer shanxing; 
    cout << shanxing.description() << endl;
 
    upgrade(&shanxing);
    cout << shanxing.description() << endl;


    return 0;
}