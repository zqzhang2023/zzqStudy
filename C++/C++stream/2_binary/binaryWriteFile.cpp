#include<iostream>
#include<fstream>
#include <string>
using namespace std;

int main(){

    string name; 
    int age; 
    ofstream outfile; 

    outfile.open("test.dat", ios::out | ios::trunc | ios::binary);

    cout << "请输入姓名:"; 
    cin >> name; 
    outfile << name << "\t";


    cout << "请输入年龄: "; 
    cin >> age; 
    outfile.write((char*)&age, sizeof(age));

    outfile.close();

    return 0;
}