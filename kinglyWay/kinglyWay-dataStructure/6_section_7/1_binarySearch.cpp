#include<iostream>
using namespace std;

int main(){

    int data[10] = {2,5,7,11,13,17,19,23,29,31};
    int findKey = 11;
    int low=0,high=9,mid;
    bool flag = false;
    while (low<=high){
        mid = (low + high)/2;
        if(data[mid]==findKey){
            cout<<"找到了! 位置为："<<mid<<endl;
            flag = true;
            break;
        }
        if(data[mid]>findKey){
            high = mid - 1;
        }
        if(data[mid]<findKey){
            low = mid + 1;
        }
    }

    if(!flag){
        cout<<"没找到"<<endl;
    }
    

    return 0;
}