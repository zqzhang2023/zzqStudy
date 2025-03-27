#include <iostream>
#include <string> 
using namespace std;
 
// 电话类 
class Tel { //虚基类 
public:
    Tel() { 
        this->number = "未知";
    } 
protected:
    string number; //电话号码;
};
 
// 座机类
class FixedLine : virtual public Tel { //虚继承
};
 
 
// 手机类 
class MobilePhone :virtual public Tel { //虚继承
};
 
// 无线座机
class WirelessTel :public FixedLine, public MobilePhone { 
public:
    void setNumber(const char *number) { 
        this->number = number; //直接访问number
    } 
 
    string getNumber() { 
        return this->number; //直接访问number
    } 
};
 
int main(void) { 
    WirelessTel phone; 
 
    phone.setNumber("123456789"); 
    cout << phone.getNumber() << endl; 
 
    return 0;
}