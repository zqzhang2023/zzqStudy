#include<iostream>
using namespace std;

int main(){
    int n = 11;
    int data[n] = {36,27,20,60,55,7,70,36,44,67,16};
    // 初始间隔为n/2，每次减半直到1
    for (int gap = n/2; gap > 0; gap /= 2) {
        //每一组内部还是使用插入排序（这里直接使用的是直接插入排序，折半的话要处理下标，很麻烦）
        for(int i = gap;i<n;i++){
            int tmp = data[i];
            int j = i;
            while(j>0&&j>=gap&&data[j-gap]>tmp){
                data[j] = data[j-gap];
                j = j - gap; 
            }
            data[j] = tmp;
        }
    }


    for(int i=0;i<11;i++){
        cout<<data[i]<<" ";
    }
    cout<<endl;
}