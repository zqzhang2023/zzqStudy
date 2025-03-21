#include<iostream>
#include<list>
using namespace std;

int main(){

    list<int> myList= {0,1,2,3,4,5,6,7,8,9};
    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;
    
    //错误！！
    //auto it = myList.begin();
    //cout<<*(it+2)<<endl;

    myList.reverse();
    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;
    myList.reverse();


    myList.push_front(6666);
    myList.push_back(8888);
    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;
    myList.pop_front();
    myList.pop_back();

    //不支持myList.begin() + 5 这种格式感觉有点不方便
    myList.insert(++myList.begin(),10);
    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;
    myList.erase(++myList.begin());
    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;

    list<int>::iterator itBegin=myList.begin();
	++ itBegin;
	list<int>::iterator itEnd=myList.begin();
	++ itEnd;
	++ itEnd;
	++ itEnd;
    myList.erase(itBegin,itEnd);//(左开右闭)

    for(auto it = myList.begin();it!=myList.end();it++){
		cout<<*it<<" ";
	}
    cout<<endl;

    return 0;
}