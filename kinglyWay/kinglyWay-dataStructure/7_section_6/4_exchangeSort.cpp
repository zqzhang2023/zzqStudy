#include<iostream>
using namespace std;

int main(){
    int n = 11;
    int data[n] = {36,27,20,60,55,7,70,36,44,67,16};
   
    for(int i=0;i<n-1;i++){//确定第i个位置的数据
        bool flag = false; //标志是否发生交换，如果没有发生交换就代表着已经有序了
        for(int j=n-1;j>i;j--){ //从最后面，一直往前面冒泡
            if(data[j]<data[j-1]){
                int temp = data[j-1];
                data[j-1] = data[j];
                data[j] = temp;
                flag = true;
            }
        }

        if(!flag){
            break;
        }
    }

    for(int i=0;i<11;i++){
        cout<<data[i]<<" ";
    }
    cout<<endl;

    return 0;
}