**2469**、**2235**、**2413**、**2160**、**2520**、**1688**、**1281**、**2427**、728、2119、509、70、
372、7、479、564、231、342、326、504、263、190、191、476、461、477、50、:693、393、172、458、258、319、405、171、168、670、233、357、400


## 2469

![alt text](images/2469.png)

想法： 直接套公式！

```cpp
class Solution {
public:
    vector<double> convertTemperature(double celsius) {
        vector<double> resultList;
        double Kelvin = celsius + 273.15;
        double Fahrenheit = celsius * 1.80 + 32.00;
        resultList.push_back(Kelvin);
        resultList.push_back(Fahrenheit);
        return resultList;
    }
};
‵‵‵
发现空间复杂度竟然不是最好的，看了一下最好的，他们是这么写的：

```cpp
class Solution
{
public:
    vector<double> convertTemperature(double celsius)
    {(),
        return {celsius + 273.15, celsius * 1.80 + 32.00};
    }
};  
```


这个有些简单的题目就不往上写了，就太简单了，比如 num1 + num2 



求最大公约数据：

辗转相减法

辗转相减法（求最大公约数），即尼考曼彻斯法，其特色是做一系列减法，从而求得最大公约数。例如 ：两个自然数35和14，用大数减去小数，(35,14)->(21,14)->(7,14)，此时，7小于14，要做一次交换，把14作为被减数，即(14,7)->(7,7)，再做一次相减，结果为0，这样也就求出了最大公约数7

```cpp
#define  _CRT_SECURE_NO_WARNINGS  
#include<stdio.h>
int main() {
    int a, b;
    int min, max;
    int temp;
    printf("请输入两个正整数:\n");
    scanf("%d %d", &a, &b);
    min = a > b ? b : a;
    max = a > b ? a : b;
    while (max - min != 0) {
        temp = max - min;
        max = temp > min ? temp : min;
        min = temp > min ? min : temp;
    }
    printf("最大公约数是:%d\n", max);
    return 0;
}
```