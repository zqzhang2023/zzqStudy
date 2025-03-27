#include<iostream>
#include<vector>
using namespace std;

int main(){

	vector<int> myVector = {0,1,2,3,4,5,6,7,8,9};
    vector<int> myVectorCopy = {10,11,12,13,14,15};
    cout<<"myVector 的元素个数: "<<myVector.size()<<endl;
	cout<<"myVector 容器的大小："<<myVector.capacity()<<endl;

    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.assign(2,888); //改变原来vector 中的元素个数和值(个数比变成2个，值都是888)
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.assign(myVectorCopy.begin(),myVectorCopy.end());  //用迭代器重新赋值
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    cout<<"myVector 的元素个数: "<<myVector.size()<<endl;       //返回容器中元素的个数
    cout<<"myVector 是否为空: "<<myVector.empty()<<endl;        //判断容器是否为空

    myVector.push_back(1000); //在容器尾部加入一个元素
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.pop_back();      //移除容器中最后一个元素
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.insert(myVector.begin()+1,888); //在pos位置插入一个elem元素的拷贝，返回新数据的位置。
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.erase(myVector.begin()+1);   //干掉单个元素
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;



    myVector.insert(myVector.begin()+1,5,888); //在pos位置插入n个elem数据，无返回值。
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.erase(myVector.begin()+1,myVector.begin()+6);   //干掉多个元素(左开右闭)
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;

    myVector.clear();
    cout<<"myVector 的元素个数: "<<myVector.size()<<endl;
	cout<<"myVector 容器的大小："<<myVector.capacity()<<endl;
    for(int i=0; i<myVector.size(); i++){
		cout<<myVector[i]<<" ";
	}
    cout<<endl;


    return 0;
}