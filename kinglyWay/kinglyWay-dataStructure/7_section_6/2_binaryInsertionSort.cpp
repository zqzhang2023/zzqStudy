#include<iostream>
using namespace std;

int main(){

    int data[11] = {36,27,20,60,55,7,70,36,44,67,16};

    for(int i=1;i<11;i++){
        int key = data[i];
        int low = 0,high = i-1;
        //通过折半查找找到插入位置
        while(low <= high){
            int mid = (low + high)/2;
            if(key > data[mid]){
                high = mid - 1;
            }else{
                low = mid + 1;
            }
        }

        for(int j=i;j>high+1;j--){
            data[j] = data[j-1];
        }
        data[high + 1] = key;
    }


    for(int i=0;i<11;i++){
        cout<<data[i]<<" ";
    }
    cout<<endl;


    return 0;
}