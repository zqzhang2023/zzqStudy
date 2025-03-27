#include<iostream>
#include<deque>

using namespace std;

int main(){

    deque<int> myDeque= {0,1,2,3,4,5,6,7,8,9};

    for(int i=0; i<myDeque.size(); i++){
		cout<<myDeque[i]<<" ";
	}
    cout<<endl;

    for(deque<int>::iterator it = myDeque.begin();it!=myDeque.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;

    //注意反向迭代器的使用 it++ 不是 -- 
    for(auto it = myDeque.rbegin();it != myDeque.rend();it++){
		cout<<*it<<" ";
	}
    cout<<endl;

    myDeque.push_back(8888); 
    myDeque.push_front(6666); 
    for(int i=0; i<myDeque.size(); i++){
		cout<<myDeque[i]<<" ";
	}
    cout<<endl;
    
    myDeque.pop_back();
    myDeque.pop_front();
    for(int i=0; i<myDeque.size(); i++){
		cout<<myDeque.at(i)<<" ";
	}
    cout<<endl;


    return 0;
}