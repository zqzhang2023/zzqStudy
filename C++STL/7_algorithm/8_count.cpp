#include <iostream>
#include <vector>
#include <algorithm>  // 包含算法库
#include <numeric>    // 包含accumulate
using namespace std;


int main() {
    vector<int> vec = {3, 1, 4, -2, 5, 3, 0, 7};

    // 1. count：统计值为3的元素个数
    int cnt = count(vec.begin(), vec.end(), 3);
    cout << "count(3): " << cnt << endl;  // 输出2

    // 2. count_if：统计偶数的个数
    int even_cnt = count_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    cout << "count_if(even): " << even_cnt << endl;  // 输出3（4, -2, 0）

    // 3. all_of：是否所有元素都>0？
    bool all_positive = all_of(vec.begin(), vec.end(), [](int x) { return x > 0; });
    cout << "all_of(>0): " << boolalpha << all_positive << endl;  // false（存在-2和0）

    // 4. any_of：是否存在负数？
    bool has_negative = any_of(vec.begin(), vec.end(), [](int x) { return x < 0; });
    cout << "any_of(<0): " << has_negative << endl;  // true（-2）

    // 5. none_of：是否所有元素都不等于10？
    bool no_ten = none_of(vec.begin(), vec.end(), [](int x) { return x == 10; });
    cout << "none_of(==10): " << no_ten << endl;  // true

    // 6. max/min：比较两个值
    cout << "max(5,3): " << max(5, 3) << endl;  // 5
    cout << "min(5,3): " << min(5, 3) << endl;  // 3

    // 7. max_element/min_element：找容器中的极值
    auto max_it = max_element(vec.begin(), vec.end());
    auto min_it = min_element(vec.begin(), vec.end());
    cout << "max_element: " << *max_it << endl;  // 7
    cout << "min_element: " << *min_it << endl;   // -2

    // 8. abs：绝对值
    cout << "abs(-5): " << abs(-5) << endl;  // 5

    // 9. accumulate：求和
    int sum = accumulate(vec.begin(), vec.end(), 0);
    cout << "accumulate(sum): " << sum << endl;  // 3+1+4-2+5+3+0+7 = 21

    return 0;
}