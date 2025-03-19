# C++运算法重载

C/C++的运算符，支持的数据类型，仅限于基本数据类型。

问题：

- 一头牛+一头马 = ？（牛马神兽？）

- 一个圆 +一个圆 = ？ （想要变成一个更大的圆）

- 一头牛 – 一只羊 = ？ (想要变成 4 只羊，原始的以物易物：1 头牛价值 5 只羊)

解决方案：使用运算符重载

那些可以重载

- 为了防止对标准类型进行运算符重载，

        C++规定重载运算符的操作对象至少有一个不是标准类型，而是用户自定义的类型比如不能重载 1+2 但是可以重载         
        cow + 2 和 2 + cow    // cow 是自定义的对象

- 不能改变原运算符的语法规则， 比如不能把双目运算符重载为单目运算

- 不能改变原运算符的优先级

- 不能创建新的运算符，比如 operator**就是非法的, operator*是可以的

- 不能对以下这四种运算符，使用友元函数进行重载= 赋值运算符，（）函数调用运算符，[ ]下标运算符，->通过指针访问类成员

- 不能对禁止重载的运算符进行重载

| **运算符** | **是否可重载** | **说明** |
|:-----------|:---------------|:---------|
| `.`        | 不可重载       | 成员访问运算符 |
| `::`       | 不可重载       | 作用域解析运算符 |
| `.*`       | 不可重载       | 成员指针访问运算符 |
| `->*`      | 不可重载       | 成员指针访问运算符 |
| `?:`       | 不可重载       | 条件运算符 |
| `sizeof`   | 不可重载       | 大小查询运算符 |
| `typeid`   | 不可重载       | 类型信息运算符 |
| `=`        | 可重载         | 赋值运算符 |
| `()`       | 可重载         | 函数调用运算符 |
| `[]`       | 可重载         | 下标运算符 |
| `->`       | 可重载         | 成员访问运算符 |
| `new`      | 可重载         | 动态内存分配运算符 |
| `delete`   | 可重载         | 动态内存释放运算符 |
| `+`        | 可重载         | 加法运算符 |
| `-`        | 可重载         | 减法运算符 |
| `*`        | 可重载         | 乘法运算符 |
| `/`        | 可重载         | 除法运算符 |
| `%`        | 可重载         | 取模运算符 |
| `++`       | 可重载         | 自增运算符 |
| `--`       | 可重载         | 自减运算符 |
| `==`       | 可重载         | 等于运算符 |
| `!=`       | 可重载         | 不等于运算符 |
| `>`        | 可重载         | 大于运算符 |
| `<`        | 可重载         | 小于运算符 |
| `>=`       | 可重载         | 大于等于运算符 |
| `<=`       | 可重载         | 小于等于运算符 |
| `&&`       | 可重载         | 逻辑与运算符 |
| `\|\|`       | 可重载         | 逻辑或运算符 |
| `!`        | 可重载         | 逻辑非运算符 |
| `~`        | 可重载         | 按位取反运算符 |
| `&`        | 可重载         | 按位与运算符 |
| `\|`        | 可重载         | 按位或运算符 |
| `^`        | 可重载         | 按位异或运算符 |
| `<<`       | 可重载         | 左移运算符 |
| `>>`       | 可重载         | 右移运算符 |
| `,`        | 可重载         | 逗号运算符 |


## 加减重载（顺便介绍以下重载的基本实现）

**使用成员函数重载运算符**

```cpp
#include<iostream>
#include <sstream>
using namespace std;

// 规则：
// 一斤牛肉：2斤猪肉
// 一斤羊肉：3斤猪肉

//羊
class Goat{ 
public:
    Goat(int weight){
        this->weight = weight;
    }; 
    int getWeight(void) const{
        return this->weight;
    };
private: 
    int weight = 0;
};


//猪
class Pork
{ 
public:
    Pork(int weight){
        this->weight = weight;
    }; 
    string description(void){
        stringstream ret;
        ret << weight << "斤猪肉";
        return ret.str();
    };
 
private: 
    int weight = 0;
};

//牛
class Cow { 
public:
    Cow(int weight){
        this->weight = weight;
    };
    // 参数此时定义为引用类型，更合适，避免拷贝
    Pork operator+(const Cow& cow){
        int tmp = (this->weight + cow.weight) * 2; 
        return Pork(tmp);
    }; //同类型进行运算，很频繁

    Pork operator+(const Goat& goat){
        int tmp = this->weight * 2 + goat.getWeight() * 3; 
        return Pork(tmp);
    }; //不同类型进行运算，比较少见 
private: 
    int weight = 0;
};

int main(){
    Cow c1(100);
    Cow c2(200);
    // 调用c1.operator+(c2);
    //相当于：Pork p = c1.operator+(c2); 
    Pork p = c1 + c2;
    cout << p.description() << endl;
    
    Goat g1(100);
    p = c1 + g1;
    cout << p.description() << endl;
    
    return 0;
}
```

**使用友元函数重载运算符**

```cpp
#include<iostream>
#include <sstream>
using namespace std;

// 规则：
// 一斤牛肉：2斤猪肉
// 一斤羊肉：3斤猪肉

//羊
class Goat{ 
public:
    Goat(int weight){
        this->weight = weight;
    }; 
    int getWeight(void) const{
        return this->weight;
    };
private: 
    int weight = 0;
};


//猪
class Pork
{ 
public:
    Pork(int weight){
        this->weight = weight;
    }; 
    string description(void){
        stringstream ret;
        ret << weight << "斤猪肉";
        return ret.str();
    };
 
private: 
    int weight = 0;
};

//牛
class Cow { 
public:
    Cow(int weight){
        this->weight = weight;
    };

    friend Pork operator+(const Cow& cow1, const Cow& cow2); 
    friend Pork operator+(const Cow& cow1, const Goat& goat);

private: 
    int weight = 0;
};



Pork operator+(const Cow &cow1, const Cow &cow2)
{ 
    int tmp = (cow1.weight + cow2.weight) * 2; 
 
    return Pork(tmp);
}
 
Pork operator+(const Cow& cow1, const Goat& goat)
{ 
    int tmp = cow1.weight * 2 + goat.getWeight() * 3; 
 
    return Pork(tmp);
}

int main(){
    Cow c1(100);
    Cow c2(200);
    // 调用c1.operator+(c2);
    //相当于：Pork p = c1.operator+(c2); 
    Pork p = c1 + c2;
    cout << p.description() << endl;
    
    Goat g1(100);
    p = c1 + g1;
    cout << p.description() << endl;
    
    return 0;
}
```

区别：

1.使用成员函数来实现运算符重载时，少写一个参数，因为第一个参数就是 this 指针。

两种方式的选择：

- 一般情况下，单目运算符重载，使用成员函数进行重载更方便（不用写参数）

- 一般情况下，双目运算符重载，使用友元函数更直观
  
        方便实现 a+b 和 b+a 相同的效果，成员函数方式无法实现。

        例如： 100 + cow;          只能通过友元函数来实现

              cow +100;           友元函数和成员函数都可以实现特殊情况：

- = （） [ ]  -> 不能重载为类的友元函数！！！（否则可能和 C++的其他规则矛盾），只能使用成员函数形式进行重载。

- 如果运算符的第一个操作数要求使用隐式类型转换，则必须为友元函数（成员函数方式的第一个参数是 this 指针）

- 注意： 同一个运算符重载， 不能同时使用两种方式来重载，会导致编译器不知道选择哪一个（二义性）

## 重载复制运算符=

注意：

- 注意赋值运算符重载的返回类型 和参数类型。

- 返回引用类型，便于连续赋值（链式 a = b = c）

- 参数使用引用类型， 可以省去一次拷贝

- 参数使用const, 便于保护实参不被破坏。

```cpp
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
```

## 重载复制运算符=

```cpp
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
```

## 重载运算符[ ]

```cpp
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
```

## 重载<<和>> 了解

为了更方便的实现复杂对象的输入和输出。

```cpp
//友元
friend ostream& operator<<(ostream& os, const Boy& boy);
friend istream& operator>>(istream& is, Boy& boy);

ostream& operator<<(ostream& os, const Boy& boy) {
    os << "ID:" << boy.id << "\t姓名:" << boy.name << "\t年龄:" << boy.age << "\t薪资:"
                << boy.salary << "\t黑马系数:" << boy.darkHorse;
    return os;
}
 
istream& operator>>(istream& is, Boy& boy)
{
    string name2;
 
    is >> name2 >> boy.age >> boy.salary >> boy.darkHorse;
    boy.name = (char*)malloc((name2.length()+1) * sizeof(char));
    strcpy_s(boy.name, name2.length() + 1, name2.c_str());
 
    return is;
}
```

## 普通类型 => 类类型 了解

调用对应的只有一个参数【参数的类型就是这个普通类型】的构造函数

```cpp
Boy boy1 = 10000;
// 薪资
构造函数Boy(int);
Boy boy2 = "hacker"
// 姓名
构造函数 Boy(char *);

//头文件
Boy(const char* name, int age, int salary, int darkHorse);
~Boy();
Boy(int salary);
Boy(const char* name);

//cpp文件
Boy::Boy(int salary)
{
    const char *defaultName = "未命名";
    name = new char[strlen(defaultName) + 1];
    strcpy_s(name, strlen(defaultName) + 1, defaultName);
    age = 0;
    this->salary = salary;
    darkHorse = 0;
    this->id = ++LAST_ID;
}
 
Boy::Boy(const char* name) {
    this->name = new char[strlen(name) + 1];
    strcpy_s(this->name, strlen(name) + 1, name);
    age = 0;
    this->salary = 0;
    darkHorse = 0;
    this->id = ++LAST_ID;
}
```

## 类类型 => 普通类型 了解

调用特殊的运算符重载函数，类型转换函数，不需要写返回类型

类型转换函数：operator 普通类型 ( )

```cpp
Boy boy1(“hacker”, 28, 10000, 5);
int power = boy1; // power();
char *name = boy1; // “hacker

Boy::operator int() const
{
    return power();
}
 
Boy::operator char* () const
{
    return name;
}
//头
// 特殊的运算符重载：类型转换函数，不需要写返回类型
operator int() const;
operator char* () const;


//cpp
Boy::operator int() const
{
    return power();
}
 
Boy::operator char* () const
{
    return name;
}
```





