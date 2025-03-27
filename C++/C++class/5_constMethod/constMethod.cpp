#include<iostream>
using namespace std;

class Human { 
public:
    Human();
    void description() const; //注意，const的位置
    ~Human();
     
private:
    const string bloodType;
    static int count;

};

int Human::count = 0;
//使用初始化列表，对const数据成员初始化
Human::Human():bloodType("A"){
    count++;
}

//注意，const的位置
void Human::description() const { 
    cout << " bloodType:" << bloodType << endl; //其他成员函数可以“读”const变量
}

Human::~Human(){
    count--;
}

int main(){

    Human h1;
 
    h1.description();

    return 0;
}

