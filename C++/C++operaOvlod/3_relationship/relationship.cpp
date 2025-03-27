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

    int power() const{
        // 薪资* 黑马系数 + (100 - 年龄） * 1000
        int value = salary * darkHorse + (100 - age) * 100;
        return value;
    };

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
 
    string description(){

        std::stringstream ret;
 
        ret << "ID:" << id << "\t姓名:" << name << "\t年龄:" << age << "\t薪资:"
            << salary << "\t黑马系数:" << darkHorse;
     
        return ret.str();
    };

    bool operator<(const Boy& boy){
        return this->power() > boy.power();
    };
    bool operator>(const Boy& boy){
        return this->power() < boy.power();
    };
    bool operator==(const Boy& boy){
        return this->power() == boy.power();
    };
};
// static只能在外面初始化 const才能在里面初始化
int Boy::LAST_ID = 0;

int main()
{
    Boy boy1("hacker", 38, 58000, 5); 
    Boy boy2("Jack", 25, 50000, 10);

    if (boy1 > boy2) { 
        std::cout << "选择boy1" << std::endl;
    }
    else if (boy1 == boy2) { 
        std::cout << "难以选择" << std::endl;
    }else{
        std::cout << "选择boy2" << std::endl;
    }

    return 0;
}