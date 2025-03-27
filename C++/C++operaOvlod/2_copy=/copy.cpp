#include<iostream>
#include<string>
#include <string.h>
#include <sstream>
using namespace std;


class Boy{
    
private:
    char* name; 
    int age; 
    int salary;
    int darkHorse; //黑马值，潜力系数 
    unsigned int id; // 编号 
    static int LAST_ID;

public:
    Boy(const char* name=NULL, int age=0, int salary=0, int darkHorse=0){
        if (!name) { 
            name = "未命名";
        }
        this->name = new char[strlen(name) + 1]; 
        strcpy(this->name, name);
        this->age = age; 
        this->salary = salary; 
        this->darkHorse = darkHorse; 
        this->id = ++LAST_ID;
    };

    ~Boy(){
        if (name) { 
            delete name;
        }
    };
    Boy& operator=(const Boy& boy){
        if (name) { 
            delete name; //释放原来的内存
        }
        name = new char[strlen(boy.name) + 1]; //分配新的内存 
        strcpy(name,boy.name);
        this->age = boy.age; 
        this->salary = boy.salary; 
        this->darkHorse = boy.darkHorse;
        this->id = boy.id;

        return *this;
    }; 
 
    string description(){

        std::stringstream ret;
 
        ret << "ID:" << id << "\t姓名:" << name << "\t年龄:" << age << "\t薪资:"
            << salary << "\t黑马系数:" << darkHorse;
     
        return ret.str();
    };
};
// static只能在外面初始化 const才能在里面初始化
int Boy::LAST_ID = 0;

int main()
{


    Boy boy1("hacker", 38, 58000, 10); 
    Boy boy2, boy3;
 
    std::cout << boy1.description() << std::endl; 
    std::cout << boy2.description() << std::endl; 
    std::cout << boy3.description() << std::endl;
    
    //链式
    boy3 = boy2 = boy1; 
 
    std::cout << boy2.description() << std::endl; 
    std::cout << boy3.description() << std::endl;

    return 0;
}