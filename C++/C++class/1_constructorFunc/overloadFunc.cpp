#include<iostream>
#include<string>
using namespace std;

class Human {
private:
    string name; 
    int age;
    int salary;

public:
    Human(); //构造函数

    Human(int age, int salary); //重载构造函数
    
    void eat(); //方法， “成员函数”
    void sleep(); 
    void play(); 
    void work();
 
    string getName(); 
    int getAge(); 
    int getSalary();

};


Human::Human() { 
    name = "无名氏"; 
    age = 18; 
    salary = 30000;
}


Human::Human(int age, int salary) {
    cout << "调用自定义的构造函数" << endl; 
    this->age = age;	//this 是一个特殊的指针，指向这个对象本身 
    this->salary = salary;
    name = "无名";
}
 
void Human::eat() {
    cout << "吃炸鸡，喝啤酒！" << endl;
}
 
void Human::sleep() { 
    cout << "我正在睡觉!" << endl;
}
 
void Human::play() { 
    cout << "我在唱歌! " << endl; }
 
void Human::work() { 
    cout << "我在工作..." << endl;
}
 
string Human::getName() { 
    return name;
}
 
int Human::getAge() { 
    return age;
}
 
int Human::getSalary() { 
    return salary;
} 


int main(){

    Human	h1(25, 35000);	// 使用自定义的默认构造函数
 
    cout << "姓名：" << h1.getName() << endl; 
    cout << "年龄: " << h1.getAge() << endl; 
    cout << "薪资：" << h1.getSalary() << endl;

    return 0;
}