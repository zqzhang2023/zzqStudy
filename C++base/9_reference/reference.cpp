#include<iostream>
using namespace std;

int main(){

    int i = 10;
	int &j = i;
	cout << "i = " << i << " j = " << j << endl;
	cout << "i的地址为 " << &i << endl;
	cout << "j的地址为 " << &j << endl;


    return 0;
}