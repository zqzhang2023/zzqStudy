#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>  // for swap and move
#include <memory>   // for unique_ptr

using namespace std;

int main() {
    // 1. swap 交换两个变量
    int a = 10, b = 20;
    swap(a, b);
    cout << "After swap: a=" << a << ", b=" << b << endl;  // a=20, b=10

    // 2. swap_ranges 交换两个容器的部分元素
    vector<int> vec1{1, 2, 3, 4, 5};
    vector<int> vec2{10, 20, 30, 40, 50};
    swap_ranges(vec1.begin(), vec1.begin() + 3, vec2.begin());  // 交换前3个元素
    cout << "vec1: ";
    for (auto x : vec1) cout << x << " ";  // 10 20 30 4 5
    cout << "\nvec2: ";
    for (auto x : vec2) cout << x << " ";  // 1 2 3 40 50

    // 3. reverse 反转容器
    reverse(vec1.begin(), vec1.end());
    cout << "\nReversed vec1: ";
    for (auto x : vec1) cout << x << " ";  // 5 4 30 20 10

    // 4. rotate 旋转元素（将中间元素移动到起始位置）
    vector<string> words{"A", "B", "C", "D", "E"};
    rotate(words.begin(), words.begin() + 2, words.end());  // 将C移到开头
    cout << "\nRotated words: ";
    for (auto& s : words) cout << s << " ";  // C D E A B

    // 5. move 转移资源所有权（结合智能指针）
    unique_ptr<int> ptr1 = make_unique<int>(42);
    unique_ptr<int> ptr2 = move(ptr1);
    cout << "\nptr2 value: " << *ptr2 << (ptr1 ? " (ptr1 valid)" : " (ptr1 empty)");

    // 6. unique 去重（需先排序）
    vector<int> nums{5, 2, 2, 3, 3, 3, 1};
    sort(nums.begin(), nums.end());
    auto last = unique(nums.begin(), nums.end());
    nums.erase(last, nums.end());
    cout << "\nUnique nums: ";
    for (auto x : nums) cout << x << " ";  // 1 2 3 5
    cout<<endl;

    return 0;
}