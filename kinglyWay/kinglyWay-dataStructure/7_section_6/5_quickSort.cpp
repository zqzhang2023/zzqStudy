#include<iostream>
using namespace std;

int partition(int data[],int low,int high){
    int pivot = data[low];
    while(low<high){
        //注意这里一定要先操作high 因为 low位置是空的插槽
        while(low<high&&data[high]>=pivot){
            high--;
        }
        data[low] = data[high]; //high位置空出来了插槽
        while(low<high&&data[low]<=pivot){
            low++;
        }
        data[high] = data[low]; //low位置空出来了插槽
    }
    //此时low=high了，就是要分割的位置
    data[low] = pivot;
    return low;
}

void quickSort(int data[],int low,int high){
    if(low<high){
        int pivotpos = partition(data,low,high);
        quickSort(data,low,pivotpos-1);
        quickSort(data,pivotpos+1,high);
    }
}

int main(){
    int n = 11;
    int data[n] = {36,27,20,60,55,7,70,36,44,67,16};
    quickSort(data,0,n-1);

    for(int i=0;i<n;i++){
        cout<<data[i]<<" ";
    }
    cout<<endl;

    return 0;
}