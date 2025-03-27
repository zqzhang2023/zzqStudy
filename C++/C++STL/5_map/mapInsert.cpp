#include<iostream>
#include<map>
using namespace std;

int main(){

    map<int,string> mapStu;

    mapStu.insert( pair<int,string>(1,"张三") );    //往容器插入元素，返回pair<iterator,bool>
    //方式一、通过pair的方式插入对象
    mapStu.insert(  pair<int,string>(1,"张三")  );
    //方式二、通过pair的方式插入对象
    mapStu.insert(make_pair(2, "李四"));
    //方式三、通过value_type的方式插入对象
    mapStu.insert(  map<int,string>::value_type(3,"王五")  );
    //方式四、通过数组的方式插入值
    mapStu[4] = "赵六";
    mapStu[5] = "小七";

    for(auto it=mapStu.begin();it!=mapStu.end();it++){
        cout<<it->first<<" "<<it->second<<endl;
    }



    return 0;
}