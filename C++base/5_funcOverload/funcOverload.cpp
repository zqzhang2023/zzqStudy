#include <iostream>
using namespace std;

int add(int x, int y){
	return x + y;
}

double add(double x, double y){
	return x + y;
}

int add(int x, int y, int z){
	return x + y + z;
}

int main() 
{
	int a = 3, b = 5, c = 7;
	double x = 10.334, y = 8.9003;
	cout << add(a, b) << endl;
	cout << add(x, y) << endl;
	cout << add(a, b, c) << endl;
	return 0;
}
