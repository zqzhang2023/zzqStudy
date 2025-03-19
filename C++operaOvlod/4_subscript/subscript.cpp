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

        stringstream ret;
 
        ret << "ID:" << id << "\t姓名:" << name << "\t年龄:" << age << "\t薪资:"
            << salary << "\t黑马系数:" << darkHorse;
     
        return ret.str();
    };

    int operator[](string index){
        if (index == "age") { 
            return age;
        }else if(index == "salary") { 
            return salary;
        }else if(index == "darkHorse") { 
            return darkHorse;
        }
        else if(index == "power") { 
            return power();
        }else{ 
            return -1;
        }
    };

    int operator[](int index){
        if (index == 0) { 
            return age;
        }else if(index == 1) { 
            return salary;
        }else if(index == 2) {
            return darkHorse;
        }else if(index == 3) { 
            return power();
        } else { 
            return -1;
        } 
    };

};
// static只能在外面初始化 const才能在里面初始化
int Boy::LAST_ID = 0;

int main()
{

    Boy boy1("hacker", 38, 58000, 5); 
    Boy boy2("Jack", 25, 50000, 10);
    cout << "age:" << boy1["age"] << endl; 
    cout << "salary:" << boy1["salary"] << endl; 
    cout << "darkHorse:" << boy1["darkHorse"] << endl; 
    cout << "power:" << boy1["power"] << endl;
    cout << "[0]:" << boy1[0] << endl; 
    cout << "[1]:" << boy1[1] << endl; cout << "[2]:" << boy1[2] <<     endl; 
    cout << "[3]:" << boy1[3] << endl;

    return 0;
}