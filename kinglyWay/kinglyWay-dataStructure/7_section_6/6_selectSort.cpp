#include<iostream>
using namespace std;

int main(){
    int n = 11;
    int data[n] = {36,27,20,60,55,7,70,36,44,67,16};

    for(int i=0;i<n-1;i++){
        int minIndex = i;
        for(int j=i+1;j<n;j++){
            if(data[minIndex]>data[j]){
                minIndex = j;
            }
        }
        if(minIndex!=i){
            int temp = data[i];
            data[i] = data[minIndex];
            data[minIndex] = temp;
        }
    }

    for(int i=0;i<n;i++){
        cout<<data[i]<<" ";
    }
    cout<<endl;


    return 0;
}