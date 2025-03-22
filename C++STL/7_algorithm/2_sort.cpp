#include<iostream>
#include <vector>
#include <algorithm> // 包含所有排序相关函数
using namespace std;

struct Data{
    int id;
    int value;
    Data(int i,int v):id(i),value(v){};
};


int main(){
    vector<int> nums = {5, 2, 9, 1, 5, 3};
    vector<Data> items = {{1,5}, {2,2}, {3,9}, {4,1}, {5,5}};

    //sort() 示例（快速排序，非稳定）
    sort(nums.begin(),nums.end());
    cout << "sort(): ";
    for (int i = 0; i < nums.size(); i++){
        cout<<nums[i]<<" ";
    }
    cout<<endl;
    cout << "Is sorted? " <<boolalpha<<is_sorted(nums.begin(), nums.end()) << "\n\n";
    
    // stable_sort() 示例（归并排序，稳定）
    stable_sort(items.begin(), items.end(), [](const Data& a, const Data& b) {
        return a.value < b.value; // 按value升序，保持相等元素的原始id顺序
    });
    cout << "stable_sort():\n";
    for (int i = 0; i < items.size(); i++){
        cout << "ID:" << items[i].id << " Val:" << items[i].value << "\n";
    }
    cout << "Is sorted? " << is_sorted(items.begin(), items.end(),
    [](const Data& a, const Data& b) { return a.value < b.value; }) << "\n";

    //  partial_sort() 示例（堆排序，部分排序）
    vector<int> vec = {5, 2, 9, 1, 5, 3};
    partial_sort(vec.begin(), vec.begin()+3, vec.end(), greater<int>());
    cout << "partial_sort() (top3 descending): ";
    for (int i = 0; i < vec.size(); i++){
        cout<<vec[i]<<" ";
    }
    cout<<endl;


    // 4. is_sorted() 示例（检查排序状态）
    vector<int> unsorted = {3, 1, 4, 2};
    cout << "Before sorting: ";
    cout << "Is sorted? " << is_sorted(unsorted.begin(), unsorted.end()) << "\n";
    sort(unsorted.begin(), unsorted.end());
    cout << "After sorting: ";
    cout << "Is sorted? " << is_sorted(unsorted.begin(), unsorted.end()) << "\n";
    
    return 0;
}