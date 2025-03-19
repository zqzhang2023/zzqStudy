#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
using namespace std;

int main(){

    string name; 
    int age;
    //也可以使用fstream, 但是fstream的默认打开方式不截断文件长度
    ofstream outfile; 

    //打开文件
    outfile.open("test.txt", ios::out | ios::trunc);

    cout << "请输入姓名:"; 
    cin >> name; 

    cout << "请输入年龄: "; 
    cin >> age; 

    stringstream s; 
    s << "name:" << name << "\t\tage:" << age << endl; 
    outfile << s.str();

    outfile.close();

    return 0;
}