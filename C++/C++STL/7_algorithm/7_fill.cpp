#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iterator>

using namespace std;

int main() {
    // 初始化一个包含5个元素的向量，默认值为0
    vector<int> vec(5);

    // 1. 使用fill将整个容器填充为5
    fill(vec.begin(), vec.end(), 5);
    cout << "1. fill结果: ";
    for_each(vec.begin(), vec.end(), [](int x) { cout << x << " "; });
    cout << "\n\n";

    // 2. 使用fill_n修改前2个元素为10
    fill_n(vec.begin(), 2, 10);
    cout << "2. fill_n结果: ";
    for_each(vec.begin(), vec.end(), [](int x) { cout << x << " "; });
    cout << "\n\n";

    // 3. 使用generate生成随机数（0-99）
    srand(time(nullptr)); // 设置随机种子
    generate(vec.begin(), vec.end(), []() { return rand() % 100; });
    cout << "3. generate结果: ";
    for_each(vec.begin(), vec.end(), [](int x) { cout << x << " "; });
    cout << "\n\n";

    // 4. 使用generate_n生成等差数列（100, 200, 300...）
    vector<int> vec2(5, 0); // 初始化5个0
    int start = 100;
    generate_n(vec2.begin(), 3, [&start]() {
        int current = start;
        start += 100;
        return current;
    });
    cout << "4. generate_n结果: ";
    for_each(vec2.begin(), vec2.end(), [](int x) { cout << x << " "; });
    cout << "\n\n";

    // 5. 使用transform计算平方并存储到新容器
    vector<int> vec3(vec.size());
    transform(vec.begin(), vec.end(), vec3.begin(), [](int x) {
        return x * x;
    });
    cout << "5. transform结果: ";
    for_each(vec3.begin(), vec3.end(), [](int x) { cout << x << " "; });

    return 0;
}