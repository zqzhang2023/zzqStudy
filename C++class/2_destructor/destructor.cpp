#include <iostream>
#include <string>
#include <cstring>
 
using namespace std;
 
// 定义一个“人类”
class Human { 
public:
    Human();
    Human(int age, int salary);
	Human(const Human&);	//不定义拷贝构造函数，编译器会生成“合成的拷贝构造函数”

    Human& operator=(const Human &);
 
    void eat(); 
    void sleep();
    void play(); 
    void work();
 
    string getName(); 
    int getAge(); 
    int getSalary();

    void setAddr(const char *newAddr); 
    const char* getAddr();

    ~Human();
 
private:
    string name = "Unknown"; 
    int age = 28;
    int salary; 
    char *addr;
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
    addr = new char[64]; 
    strcpy(addr, "China");
}


Human::Human(const Human &man) { 
    cout << "调用自定义的拷贝构造函数" << endl; 
    age = man.age;	//this 是一个特殊的指针，指向这个对象本身 
    salary = man.salary; 
    name = man.name;

    // 深度拷贝
    addr = new char[64]; 
    strcpy(addr,man.addr);
}

Human& Human::operator=(const Human &man) { 
    cout << "调用" << __FUNCTION__ << endl; 

    if (this == &man) { 
        return *this; //检测是不是对自己赋值：比如 h1 = h1;
    }
    
    // 深拷贝
    strcpy(addr, man.addr);
 
    // 处理其他数据成员
    name = man.name; 
    age = man.age; 
    salary = man.salary;
 
    // 返回该对象本身的引用， 以便做链式连续处理，比如 a = b = c; 
    return *this;
}



void Human::eat() {
    cout << "吃炸鸡，喝啤酒！" << endl;
}
 
void Human::sleep() { 
    cout << "我正在睡觉!" << endl;
}
 
void Human::play() {     
    cout << "我在唱歌! " << endl;
}
 
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

void Human::setAddr(const char *newAddr) { 
    if (!newAddr) { 
        return;
    }
    strcpy(addr,newAddr);
}
 
const char* Human::getAddr() { 
    return addr;
}

Human::~Human(){
    cout<<"调用了析构函数！"<<endl;
    delete [] addr;
}


int main(void) {
    Human	h1(25, 35000);	// 使用自定义的默认构造函数 
    Human	h2(h1);	// 使用自定义的拷贝构造函数
 
    cout << "h1 addr:" << h1.getAddr() << endl; 
    cout << "h2 addr:" << h2.getAddr() << endl; 
    h1.setAddr("长沙");
    cout << "h1 addr:" << h1.getAddr() << endl; 
    cout << "h2 addr:" << h2.getAddr() << endl;

    
    return 0;
}

