#include<iostream>
#include<fstream>
#include <string>
using namespace std;

int main(){

    string name; 
    int age;
    ifstream infile; 

    infile.open("test.txt");

    infile >> name; 
    cout << name << "\t";
    infile >> age;
    cout << age << endl;

    infile.close();

    return 0;
}