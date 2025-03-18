#include<iostream>
#include<string>
#include <sstream>
using namespace std;

class Father{

private:
    int age; 
    string name;

public:
    Father(const char*name, int age):name(name),age(age){};
    ~Father(){};
 
    string getName(){
        return this->name;
    }
    int getAge(){
        return this->age;
    } 
    string description(){
        stringstream ret; 
        ret << "name:" << name << " age:" << age;
        return ret.str();
    }

};

//public继承 
class Son : public Father { 
private:
    string game;

public:
    Son(const char *name, int age, const char *game):Father(name, age),game(game){};
    
    ~Son(){}; 
 
    string getGame(){
        return game;
    }
    string description(){
        stringstream ret;
        // 子类的成员函数中, 不能访问从父类继承的private成员
        ret << "name:" << getName() << " age:" << getAge()
            << " game:" << game; return ret.str();
    }
};

int main(){
    Father wjl("王健林", 68);
    Son wsc("王思聪", 32, "电竞");
 
    cout << wjl.description() << endl;
    // 子类对象调用方法时, 先在自己定义的方法中去寻找, 如果有, 就调用自己定义的方法
    // 如果找不到, 就到父类的方法中去找, 如果有, 就调用父类的这个同名方法 // 如果还是找不到, 就是发生错误!
    cout << wsc.description() << endl;

    return 0;
}