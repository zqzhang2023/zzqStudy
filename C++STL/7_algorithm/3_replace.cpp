#include<iostream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <iterator>
using namespace std;

int main(){

    vector<int> vec{1, 2, 10, 4, 10, 6, 7, -3};
    string str = "Hello World! C++ STL";

    //replace：原地替换所有等于 10 的元素为 99 */
    replace(vec.begin(), vec.end(), 10, 99);
    cout << "After replace(10→99): ";

    for (int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }
    cout<<endl;

    //replace_if：替换字符串中的空格为下划线
    replace_if(str.begin(), str.end(),[](char c) { return isspace(c); }, '_');
    cout << "After replace_if(space→_): " << str << "\n";  // 输出：Hello_World!_C++_STL
    
    //replace_copy：复制并替换负数为 0（原容器不变）
    vector<int> vec_copy;
    replace_copy(vec.begin(), vec.end(), back_inserter(vec_copy), -3, 0);
    cout << "vec_copy after replace_copy(-3→0): ";
    for (int i = 0; i < vec_copy.size(); i++){
        cout << vec_copy[i] << " ";
    }
    cout<<endl;

    //replace_copy_if：复制并替换奇数为 100
    deque<int> dq;
    replace_copy_if(vec.begin(), vec.end(), back_inserter(dq),
        [](int n) { return n % 2 != 0; }, 100);
    cout << "dq after replace_copy_if(odd→100): ";
    for (int i = 0; i < dq.size(); i++){
        cout << dq[i] << " ";
    }
    cout<<endl;

    return 0;
}