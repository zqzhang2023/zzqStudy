#include<iostream>
using namespace std;

class Human { 
public:
    Human();
    static int getCount();
    ~Human();
     
private:
    static int count;
};


//对于非 const 的类静态成员，只能在类的实现文件中初始化。
int Human::count = 0;

Human::Human(){
    count++;
}

//实现的时候不用家static
int Human::getCount()
{
    return count;
}

Human::~Human(){
    count--;
}


int main(){

    cout << Human::getCount() << endl;


    Human h1;
    cout << Human::getCount() << endl;

    Human h2; 
    cout << Human::getCount() << endl;


    {
        Human h3; 
        cout << Human::getCount() << endl;
    }

    cout << Human::getCount() << endl;


    return 0;
}

