#include<iostream>
using namespace std;

void merge(int data[],int left,int right,int mid){
    int n1 = mid - left + 1;  //将中间元素归于左边了
    int n2 = right - mid;

    int leftArr[n1] = {0};
    int rightArr[n2] = {0};

    for(int i=0;i<n1;i++){
        leftArr[i] = data[i+left];
    }

    for(int j=0;j<n2;j++){
        rightArr[j] = data[j+mid+1];
    }

    //归并
    int i = 0,j=0,k=left;
    while(i<n1&&j<n2){
        if(leftArr[i]<=rightArr[j]){
            data[k] = leftArr[i];
            i++;
        }else{
            data[k] = rightArr[j];
            j++;
        }
        k++;
    }
    //把较长的那个未检测完的那个复制一下
    while(i<n1){
        data[k++] = leftArr[i++];
    }
    while(j<n2){
        data[k++] = rightArr[j++];
    }

}

void mergeSortRecursive(int data[],int left,int right){
    if(left<right){
        int mid = (left + right) / 2;
        mergeSortRecursive(data,left,mid);
        mergeSortRecursive(data,mid+1,right);
        merge(data,left,right,mid);
    }
}

// 非递归实现 //自下而上
void mergeSortIterative(int data[], int n) {
    for (int size = 1; size < n; size *= 2) {
        //注意这里为啥要left+ 2 * size，因为一次要合并俩数组，左边一个size，右边一个size
        for(int left=0;left<n;left+=2*size){    
            int mid = min(left+size-1,n-1);
            int right = min(left+2*size-1,n-1);
            if(mid<right){
                merge(data,left,right,mid);
            }
        }
    }
}

int main(){
    int n = 11;
    int data_1[n] = {36,27,20,60,55,7,70,36,44,67,16};

    mergeSortRecursive(data_1, 0, n - 1);
    cout << "递归结果: ";
    for(int i=0;i<n;i++){
        cout<<data_1[i]<<" ";
    }
    cout<<endl;

    int data_2[n] = {36,27,20,60,55,7,70,36,44,67,16};
    mergeSortIterative(data_2, n);
    cout << "递归结果: ";
    for(int i=0;i<n;i++){
        cout<<data_2[i]<<" ";
    }
    cout<<endl;

    return 0;
}