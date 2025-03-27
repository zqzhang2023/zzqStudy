#include<iostream>
#include<set>
using namespace std;

int main(){
    //默认的是less
    set<int,greater<int>> setIntA;

    setIntA.insert(5);
    setIntA.insert(1);
    setIntA.insert(2);
    setIntA.insert(3);
    setIntA.insert(4);

    set<int,greater<int>> setIntB(setIntA);
    set<int,greater<int>> setIntC;
    setIntC = setIntA;		//1 2 3 4 5 
    setIntC.insert(6);      //1 2 3 4 5 6
    setIntC.swap(setIntA);	  //交换

    for (auto it = setIntA.begin();it!=setIntA.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;

    for (auto it = setIntB.begin();it!=setIntB.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    
    for (auto it = setIntC.begin();it!=setIntC.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;

    return 0;
}