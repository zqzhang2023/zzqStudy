#include<iostream>
#include <string>

using namespace std;


class Father { 

protected:
    string lastName; //姓 
    string firstName; //名
public:
    Father(const char *lastName="无姓", const char *firstName="无名"){
        this->lastName = lastName; 
        this->firstName = firstName;
        cout<<"Father"<<endl;
    };
    void playBasketball(){
        cout << "呦呦, 我要三步上篮了!" << endl;
    };

    void dance(){
        cout << "嘿嘿, 我要跳霹雳舞!" << endl;
    };
    ~Father(){

    };
 
};



class Mother
{ 
private:
    string lastName; //姓 
    string firstName; //名 
    string food; //喜欢的食物

public:
    Mother(const char * food, const char *lastName = "无姓", const char *firstName = "无名"){
        this->food = food; 
        this->lastName = lastName; 
        this->firstName = firstName;
        cout<<"Mother"<<endl;
    };
    void dance(){
        cout << "一起跳舞吧, 一二三四, 二二三四..." << endl;
    };
    ~Mother(){

    };
 
};


class Son : public Father, public Mother { 
private:
    string game;
public:
    Son(const char *lastName, const char *firstName, const char *food, const char *game):Father(lastName, firstName),Mother(food){
        this->game = game;
        cout<<"Son"<<endl;
    };
    ~Son(){

    };
     
    void playGame(){
        cout << "一起玩" << game << "吧..." << endl;
    };

    void dance(){
        Father::dance();
        Mother::dance();
        cout << "霍霍, 我们来跳街舞吧! " << endl;
    }

};



int main(void) {
    Son wsc("川菜", "王", "思聪", "电竞");
    // 解决多重继承的二义性的方法1:
    // 使用 "类名::" 进行指定, 指定调用从哪个基类继承的方法! 
    wsc.Father::dance();         
    wsc.Mother::dance();


    // 解决多重继承的二义性的方法2:
    // 在子类中重新实现这个同名方法, 并在这个方法内部, 使用基类名进行限定,
    // 来调用对应的基类方法 
    wsc.dance(); 
    wsc.playGame();
 
    return 0;
}