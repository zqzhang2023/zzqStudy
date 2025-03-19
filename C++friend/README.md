# C++友元

使用前提：某个类需要实现某种功能，但是这个类自身，因为各种原因，无法自己实现。

需要借助于“外力”才能实现。

友元的两种使用形式：友元函数、友元类。

## 友元函数

### 使用全局函数作为友元函数

全局函数可以直接访问类内的私有成员！！！！！

```cpp
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
```

### 使用类的成员函数作为友元函数
和使用全局函数差不多，但是这里要注意声明和实现的顺序！！！

```cpp
#include<iostream>
#include <sstream>

using namespace std;

// 前向声明
class Computer;

class ComputerService { 
public:
    void upgrade(Computer* computer);
};


class Computer { 
public:
    Computer(){
        cpu = "i7";
    };
    // 使用全局函数作为友元函数
    friend void upgrade(Computer* computer);

    friend void ComputerService::upgrade(Computer* computer);

    string description(){
        stringstream ret; 
        ret << "CPU:" << cpu; 
        return ret.str();
    };
private: 
    string cpu; //CPU 芯片
};

// ComputerService成员函数的实现（此时Computer已定义，可访问私有成员）
void ComputerService::upgrade(Computer* computer) {
    computer->cpu = "i9";
}

//直接访问对象的私有数据成员！！！! ! ! ! !
void upgrade(Computer* computer) {
    computer->cpu = "i9"; //直接访问对象的私有数据成员！！！
}

int main(){

    Computer shanxing; 
    ComputerService service;
    std::cout << shanxing.description() << std::endl;
    service.upgrade(&shanxing);
    std::cout << shanxing.description() << std::endl;


    return 0;
}
```

功能上，这两种形式，都是相同，应用场合不同。

一个是，使用普通的全局函数，作为自己的朋友，实现特殊功能。

一个是，使用其他类的成员函数，作为自己的朋友，实现特殊功能。


## 友元类

**为什么要使用友元类**

一个独立的咨询师， 给其他企业做服务时，这个咨询师作为企业的“友元函数”即可。

一个大型的咨询服务公司，比如 IBM（IT 事务）, 普华永道（会计事务），给其他企业做服务时，使用友元函数就不是很方便了，因为需要设计很多友元函数，不方便。

解决方案：使用“友元类”

**友元类的作用**

如果把 A 类作为 B 类的友元类，那么 A 类的所有成员函数【在 A 类的成员函数内】，就可以直接访问【使用】B 类的私有成员。

即，友元类可以直接访问对应类的所有成员！！！

```cpp
#include<iostream>
#include <sstream>

using namespace std;

// 前向声明
class ComComputerServiceputer;

class Computer { 
public:
    Computer(){
        cpu = "i7";
    };
    string description(){
        stringstream ret; 
        ret << "CPU:" << cpu; 
        return ret.str();
    };
private: 
    string cpu; //CPU 芯片

    friend class ComputerService;

};

// 友元类可以直接访问声明类之中的所有的参数，包括私有参数
class ComputerService { 
public:
    void upgrade(Computer* computer){
        computer->cpu = "i9";
    }; 
    void clean(Computer* computer){
        std::cout << "正在对电脑执行清理[CPU:"
        << computer->cpu << "]..."
        << std::endl;
    }; //计算机清理 
    void kill(Computer* computer){
        std::cout << "正在对电脑执行杀毒[CPU:"
        << computer->cpu << "]..."
        << std::endl;
    }; //杀毒
};


int main(){

    Computer shanxing; 
    ComputerService service;
 
    std::cout << shanxing.description() << std::endl;
    service.upgrade(&shanxing); 
    service.clean(&shanxing); 
    service.kill(&shanxing);
    std::cout << shanxing.description() << std::endl;


    return 0;
}
```

**注意**：友元类，和友元函数，使用 friend 关键字进行声明即可，与访问权限无关，所以，可以放在 private/pulic/protected 任意区域内

说实话，这有点破坏class良好的封装性了，如果是我的话，除非必须要用，否则尽量不用



