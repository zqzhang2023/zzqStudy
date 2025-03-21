#include<iostream>
#include<set>
using namespace std;

int main(){

    set<int>  setInt;
	for(int i=5; i>0; i--){
		pair<set<int>::iterator, bool> ret = setInt.insert(i);
		if(ret.second){
			cout<<"插入 "<<i<<" 成功！"<<endl;
            cout<<*ret.first<<endl;
		}else {
			cout<<"插入 "<<i<<" 失败！"<<endl;
		}
	}

    for(int i=0; i<5; i++){
		pair<set<int>::iterator, bool> ret = setInt.insert(i);
		if(ret.second){
			cout<<"插入 "<<i<<" 成功！"<<endl;
            cout<<*ret.first<<endl;
		}else {
			cout<<"插入 "<<i<<" 失败！"<<endl;
		}
	}

    return 0;
}