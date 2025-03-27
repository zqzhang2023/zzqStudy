#include<iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

int main(){
    vector<int> data = {5, 3, 3, 7, 2, 8, 3, 9, 1, 3, 8, 6, 4};
    vector<int> subset = {3, 9, 1};

    // find() 函数，返回迭代器（指针|位置），其指向的是在 [first, last) 区域内查找到的第一个目标元素；如果查找失败，则该迭代器的指向和 last 相同。
    auto it_find = find(data.begin(), data.end(),8);
    if (it_find != data.end()){
        cout << "find(8) at pos: " << distance(data.begin(), it_find) << endl; // 输出: 5
    }

    //find_if()、find_if_not() 函数，根据指定的【查找规则】进行查找，在指定区域内查找第一个符合该函数要求（使函数返回 true）的元素
    auto it_find_if = find_if(data.begin(), data.end(), [](int x){return x > 7;});
    if (it_find_if != data.end()){
        cout << "find_if(>7): " << *it_find_if << " at pos: " << distance(data.begin(), it_find_if) << endl; // 输出: 7 at pos:5
    }
    
    // find_first_of() 函数，查找范围A中第一个与范围B中【任一元素】等价的元素的位置
    auto it_first_of = find_first_of(data.begin(), data.end(), subset.begin(), subset.end());
    if (it_first_of != data.end()){
        cout << "find_first_of(subset) at pos: " << distance(data.begin(), it_first_of) << endl; // 输出:1
    }

    // adjacent_find() 函数，查找两个相邻（adjacent）的等价（identical）元素
    auto it_adj = adjacent_find(data.begin(), data.end());
    if (it_adj != data.end()){
        cout << "adjacent_find: " << *it_adj << " at pos: " << distance(data.begin(), it_adj) << endl; // 输出:3 at pos:5
    }
    
    // 查找子串
    auto it_search = search(data.begin(), data.end(), subset.begin(), subset.end());
    if (it_search != data.end()){
        cout << "search(subset) at pos: " << distance(data.begin(), it_search) << endl; // 输出:6
    }

    // 对有序区间操作
    sort(data.begin(), data.end()); // 排序为:1 2 3 3 3 3 4 5 6 7 8 8 9

    // binary_search: 检查元素是否存在
    bool exists = binary_search(data.begin(), data.end(), 6);
    cout << "binary_search(6): " << boolalpha << exists << endl; // 输出:true

    //lower_bound: 第一个>=5的位置
    auto it_low = lower_bound(data.begin(), data.end(), 5);
    cout << "lower_bound(5) at pos: " << distance(data.begin(), it_low) << endl; // 输出:7

    //upper_bound: 第一个>8的位置
    auto it_high = upper_bound(data.begin(), data.end(), 8);
    cout << "upper_bound(8) at pos: " << distance(data.begin(), it_high) << endl; // 输出:12

    //equal_range: 查找3的范围
    auto eq_pair = equal_range(data.begin(), data.end(), 3);
    cout << "equal_range(3): [" << distance(data.begin(), eq_pair.first) 
         << "," << distance(data.begin(), eq_pair.second) << ")" << endl; // 输出:[2,5)
    
}