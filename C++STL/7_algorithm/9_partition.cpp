#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;

bool isOdd(int x) { return x % 2 != 0; } // 分区条件：奇数在前，偶数在后

int main() {
    // 初始数据
    vector<int> data{3,1,4,5,8,6,7,2};
    vector<int> copy1, copy2;

    // 1. partition 基础分区
    auto it = partition(data.begin(), data.end(), isOdd);
    cout << "After partition: ";
    copy(data.begin(), data.end(), ostream_iterator<int>(cout, " "));
    cout << "\nPartition point: " << *it << "\n\n";

    // 2. is_partitioned 验证分区
    cout << "Is partitioned? " 
              << (is_partitioned(data.begin(), data.end(), isOdd) ? "Yes" : "No")
              << "\n\n";

    // 3. stable_partition 稳定分区（保留原始顺序）
    data = {3,1,4,5,8,6,7,2}; // 重置数据
    auto it_stable = stable_partition(data.begin(), data.end(), isOdd);
    cout << "After stable_partition: ";
    copy(data.begin(), data.end(), ostream_iterator<int>(cout, " "));
    cout << "\nStable partition point: " << *it_stable << "\n\n";

    // 4. partition_point 查找分界点
    auto pp = partition_point(data.begin(), data.end(), isOdd);
    cout << "Partition point via partition_point: " << *pp << "\n\n";

    // 5. partition_copy 分区复制
    partition_copy(data.begin(), data.end(),
                       back_inserter(copy1), // 奇数容器
                       back_inserter(copy2), // 偶数容器
                       isOdd);
    cout << "Odd elements: ";
    copy(copy1.begin(), copy1.end(), ostream_iterator<int>(cout, " "));
    cout << "\nEven elements: ";
    copy(copy2.begin(), copy2.end(), ostream_iterator<int>(cout, " "));
    cout << "\n\n";

    // 6. next_permutation 下一个排列
    vector<int> perm{1,2,3};
    cout << "All permutations (ascending):\n";
    do {
        copy(perm.begin(), perm.end(), ostream_iterator<int>(cout, " "));
        cout << " → ";
    } while (next_permutation(perm.begin(), perm.end()));
    cout << "\n\n";

    // 7. prev_permutation 上一个排列
    sort(perm.rbegin(), perm.rend()); // 先降序排序
    cout << "All permutations (descending):\n";
    do {
        copy(perm.begin(), perm.end(), ostream_iterator<int>(cout, " "));
        cout << " → ";
    } while (prev_permutation(perm.begin(), perm.end()));
    cout << "\n\n";

    // 8. is_permutation 验证排列
    vector<int> test1{1,2,3}, test2{3,2,1}, test3{1,2,4};
    cout << "Is permutation (1-2-3 vs 3-2-1): " 
              << boolalpha << is_permutation(test1.begin(), test1.end(), test2.begin())
              << "\nIs permutation (1-2-3 vs 1-2-4): "
              << is_permutation(test1.begin(), test1.end(), test3.begin()) << "\n";

    return 0;
}