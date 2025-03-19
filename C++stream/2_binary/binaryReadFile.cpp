#include<iostream>
#include<fstream>
#include <string>
using namespace std;

int main(){

    string name; 
    int age;
    ifstream infile; 

    infile.open("test.dat", ios::in | ios::binary);

    infile >> name; 
    cout << name << "\t";

    // 跳过中间的制表符
    char tmp;
    infile.read(&tmp, sizeof(tmp));


    infile.read((char*)&age, sizeof(age));
    cout << age << endl;

    infile.close();

    return 0;
}