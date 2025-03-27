#include <iostream>
using namespace std;

int main(){

    //异常处理，一定要注意
    int* ptr = nullptr;
    try {
        ptr = new int[100];
    } catch (const std::bad_alloc& e) {
        cerr << "Memory allocation failed: " << e.what() << endl;
    }

    delete[] ptr;

    return 0;
}