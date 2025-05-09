#include<iostream>
using namespace std;

int main(){

    int data_1[11] = {36,27,20,60,55,7,70,36,44,67,16};

    for(int i=1;i<11;i++){
        for(int j=i;j>0;j--){
            if(data_1[j]<data_1[j-1]){
                int temp = data_1[j-1];
                data_1[j-1] = data_1[j];
                data_1[j] = temp;
            }
        }
    }

    for(int i=0;i<11;i++){
        cout<<data_1[i]<<" ";
    }
    cout<<endl;


    int data_2[11] = {36,27,20,60,55,7,70,36,44,67,16};
    for(int i=1;i<11;i++){
        int key = data_2[i];
        int j = i;
        while(j > 0&& key > data_2[j-1]){
            data_2[j] = data_2[j-1];
            j--;
        }
        data_2[j] = key;
    }

    for(int i=0;i<11;i++){
        cout<<data_2[i]<<" ";
    }
    cout<<endl;


    return 0;
}