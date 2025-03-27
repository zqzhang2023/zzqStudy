#include<iostream>
#include <set>
using namespace std;

class Student{
private:
    //主要是为了比较，name啥的属性就不写了
    int age;
public:

    Student(int age){
        this->age = age;
    }

    int getAge() const{
        return this->age;
    }

    bool operator < (const Student & right) const{
        return this->getAge() < right.getAge();
    }

    bool operator > (const Student & right) const{
        return this->getAge() > right.getAge();
    }

};

int main(){

    set<Student>  setStuLess;  //相当于 set<student,less<student>> （student之中定义了比较函数）
    setStuLess.insert(Student(20));
    setStuLess.insert(Student(18));
    setStuLess.insert(Student(19));
    setStuLess.insert(Student(19));   //这个插入无效，但是没有报错，这里一定要注意
    for(auto it =  setStuLess.begin();it!=setStuLess.end();it++){
        cout<<it->getAge()<<" ";
    }
    cout<<endl;


    set<Student,greater<Student>>  setStuGreater;  //相当于 set<student,less<student>> （student之中定义了比较函数）
    setStuGreater.insert(Student(20));
    setStuGreater.insert(Student(18));
    setStuGreater.insert(Student(19));
    for(auto it =  setStuGreater.begin();it!=setStuGreater.end();it++){
        cout<<it->getAge()<<" ";
    }
    cout<<endl;

    return 0;
}