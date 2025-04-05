#include<iostream>
using namespace std;

//数组，需要维护的节点个数，以及需要维护的当前节点
void heapifyMax(int data[],int n,int index){ 
    int largest = index;
    int lson = index * 2 + 1;
    int rson = index * 2 + 2;

    if(lson<n&&data[largest]<data[lson]){
        largest = lson;
    }
    if(rson<n&&data[largest]<data[rson]){
        largest = rson;
    }
    //等于的话表示当前堆是正常的不需要维护
    if(largest!=index){ 
        //交换
        int temp = data[index];
        data[index] = data[largest];
        data[largest] = temp;
        //递归剩下的，因为index调整好之后，可能会导致其子结点破环堆的性质
        heapifyMax(data,n,largest);
    }
}

void heapSortMax(int data[], int n) {
    //建立堆
    for(int i=n/2+1;i>=0;i--){   //从后往前，找到第一个有子结点的节点
        heapifyMax(data,n,i);
    }

    //排序
    for(int i=n-1;i>0;i--){
        //交换 将最大值放到最后面
        int temp = data[i];
        data[i] = data[0];
        data[0] = temp;
        //重新整理堆 //只剩下i个元素了
        heapifyMax(data,i,0);
    }
}

void heapifyMin(int data[],int n,int index){ 
    int minimum = index;
    int lson = index * 2 + 1;
    int rson = index * 2 + 2;

    if(lson<n&&data[minimum]>data[lson]){
        minimum = lson;
    }

    if(rson<n&&data[minimum]>data[rson]){
        minimum = rson;
    }

    if(minimum!=index){
        int temp = data[index];
        data[index] = data[minimum];
        data[minimum] = temp;
        heapifyMin(data,n,minimum);
    }

}

void heapSortMin(int data[], int n) {
    //建堆
    for(int i = n/2+1;i>=0;i--){
        heapifyMin(data,n,i);
    }
    //排序
    for(int i=n-1;i>0;i--){
        int temp = data[i];
        data[i] = data[0];
        data[0] = temp;
        heapifyMin(data,i,0);
    }
}


int main(){
    int n = 11;
    int data_1[n] = {36,27,20,60,55,7,70,36,44,67,16};
    int data_2[n] = {36,27,20,60,55,7,70,36,44,67,16};

    // 大顶堆排序（升序）
    heapSortMax(data_1, n);
    cout << "大顶堆排序结果（升序）: ";
    for(int i=0;i<n;i++){
        cout<<data_1[i]<<" ";
    }
    cout<<endl;

    // 小顶堆排序（降序）
    heapSortMin(data_2, n);
    cout << "小顶堆排序结果（降序）: ";
    for(int i=0;i<n;i++){
        cout<<data_2[i]<<" ";
    }
    cout<<endl;

    return 0;
}