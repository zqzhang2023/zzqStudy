#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

int main(){

    vector<int> vec = {1, 2, 3, 2, 5, 4, 2, 6};
    for(auto it = vec.begin();it!=vec.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;

    // remove 示例：删除所有值为 2 的元素
    auto new_end = remove(vec.begin(), vec.end(), 2);
    cout << "remove 后逻辑范围: ";
    for(auto it = vec.begin();it!=new_end;it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    cout << "实际容器内容: ";
    for(auto it = vec.begin();it!=vec.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;

    vec.erase(new_end, vec.end()); 
    cout << "物理删除后容器: ";
    for(auto it = vec.begin();it!=vec.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;


    //remove_if 示例：删除所有偶数
    vec = {1, 2, 3, 4, 5, 6, 7, 8};  // 重置数据
    new_end = remove_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    cout << "remove_if 后逻辑范围: ";
    for(auto it = vec.begin();it!=new_end;it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    cout << "物理删除后容器: ";
    vec.erase(new_end, vec.end());
    for(auto it = vec.begin();it!=vec.end();it++){
        cout<<*it<<" ";
    }
    cout << endl;


    //remove_copy 示例：复制时跳过 3
    vector<int> dest1;
    vec = {1, 3, 5, 3, 7, 9};  // 重置数据
    remove_copy(vec.begin(), vec.end(), back_inserter(dest1), 3);
    cout << "remove_copy 结果（跳过3）: ";
    for(auto it = dest1.begin();it!=dest1.end();it++){
        cout<<*it<<" ";
    }
    cout << endl;

    // remove_copy_if 示例：复制时跳过长度超过3的字符串
    vector<string> str_vec = {"cat", "apple", "dog", "elephant", "bee"};
    vector<string> dest2;
    remove_copy_if(str_vec.begin(), str_vec.end(), back_inserter(dest2),
        [](const string& s) { return s.length() > 3; });
    cout << "remove_copy_if 结果（跳过长度>3的字符串）: ";
    for(auto it = dest2.begin();it!=dest2.end();it++){
        cout<<*it<<" ";
    }
    cout << endl;




    return 0;
}

 