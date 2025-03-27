#include<iostream>
using namespace std;

void defaultParaFunc(int a = 5,int b = 10){
    cout<<"a = "<<a<<" b = "<<b<<endl;
}

int main(){

    defaultParaFunc();   //默认 a=5 b=10
    defaultParaFunc(25); //a=25 默认 b=10
    defaultParaFunc(25,30); // a=25 b=30
    
    return 0;
}