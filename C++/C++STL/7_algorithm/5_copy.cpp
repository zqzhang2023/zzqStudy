#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;

int main() {
    vector<int> src = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 1. copy: 复制整个序列
    vector<int> dest1;
    copy(src.begin(), src.end(), back_inserter(dest1));
    cout << "copy 结果: ";
    for (int x : dest1) cout << x << " ";
    cout << "\n\n";

    // 2. copy_n: 复制前5个元素
    vector<int> dest2;
    copy_n(src.begin(), 5, back_inserter(dest2));
    cout << "copy_n(5) 结果: ";
    for (int x : dest2) cout << x << " ";
    cout << "\n\n";

    // 3. copy_if: 复制偶数
    vector<int> dest3;
    copy_if(src.begin(), src.end(), back_inserter(dest3),
                 [](int x) { return x % 2 == 0; });
    cout << "copy_if(偶数) 结果: ";
    for (int x : dest3) cout << x << " ";
    cout << "\n\n";

    // 4. copy_backward: 从后向前复制到目标容器末尾
    vector<int> dest4(8, 0); // 目标容器需预分配空间
    copy_backward(src.begin() + 2, src.begin() + 6, dest4.end());
    cout << "copy_backward 结果: ";
    for (int x : dest4) cout << x << " ";
    cout << "\n\n";

    // 5. reverse_copy: 逆序复制
    vector<int> dest5;
    reverse_copy(src.begin(), src.end(), back_inserter(dest5));
    cout << "reverse_copy 结果: ";
    for (int x : dest5) cout << x << " ";
    cout << "\n\n";

    // 6. merge: 合并两个有序序列
    vector<int> sorted1 = {2, 4, 6, 8};
    vector<int> sorted2 = {1, 3, 5, 7};
    vector<int> merged;
    merge(sorted1.begin(), sorted1.end(), 
              sorted2.begin(), sorted2.end(), 
              back_inserter(merged));
    cout << "merge 结果: ";
    for (int x : merged) cout << x << " ";
    cout << "\n\n";

    // 7. inplace_merge: 原地合并已排序的子序列
    vector<int> vec = {1, 3, 5, 2, 4, 6};
    sort(vec.begin(), vec.begin() + 3); // 前半部分排序
    sort(vec.begin() + 3, vec.end());    // 后半部分排序
    inplace_merge(vec.begin(), vec.begin() + 3, vec.end());
    cout << "inplace_merge 结果: ";
    for (int x : vec) cout << x << " ";
    cout << "\n\n";
    
    return 0;
}