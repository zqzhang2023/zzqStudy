#include <iostream>
#include <string.h>
using namespace std;

class Father {
public:
    Father(const char* addr ="中国"){
        cout << "执行了 Father 的构造函数" << endl;
        int len = strlen(addr) + 1;
        this->addr = new char[len];
        strcpy(this->addr, addr);
    }
    // 把 Father 类的析构函数定义为 virtual 函数时，
    // 如果对 Father 类的指针使用 delete 操作时，
    // 就会对该指针使用“动态析构”：
    // 如果这个指针，指向的是子类对象，
    // 那么会先调用该子类的析构函数，再调用自己类的析构函数
    virtual ~Father(){
        cout << "执行了 Father 的析构函数" << endl;
        if (addr) {
            delete addr;
            addr = NULL;
        }
    }
private:
    char* addr;
};

class Son :public Father {
public:
    Son(const char *game="吃鸡", const char *addr="中国")
    :Father(addr){
        cout << "执行了 Son 的构造函数" << endl;
        int len = strlen(game) + 1;
        this->game = new char[len];
        strcpy(this->game, game);
    }
 
~Son(){
    cout << "执行了 Son 的析构函数" << endl;
    if (game) {
        delete game;
        game = NULL;
    }
}
 
private:
    char* game;
};
 
int main(void) {
    cout << "----- case 1 -----" << endl;
    Father* father = new Father();
    delete father;
    cout << "----- case 2 -----" << endl;
    Son* son = new Son();
    delete son;
    cout << "----- case 3 -----" << endl;
    father = new Son();
    delete father;
 
    return 0;
}