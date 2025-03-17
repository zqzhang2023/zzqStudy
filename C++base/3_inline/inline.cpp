#include<iostream>
using namespace std;

inline double circle(double r){
    double PI = 3.14;
    return PI * r * r;
}

int main(){

    for (int i = 0; i < 3; i++){
        cout <<"r = "<< i <<" area = " << circle(i) <<endl;
    }

    return 0;
}