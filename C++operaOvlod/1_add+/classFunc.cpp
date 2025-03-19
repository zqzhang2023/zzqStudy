#include<iostream>
#include <sstream>
using namespace std;

// 规则：
// 一斤牛肉：2斤猪肉
// 一斤羊肉：3斤猪肉

//羊
class Goat{ 
public:
    Goat(int weight){
        this->weight = weight;
    }; 
    int getWeight(void) const{
        return this->weight;
    };
private: 
    int weight = 0;
};


//猪
class Pork
{ 
public:
    Pork(int weight){
        this->weight = weight;
    }; 
    string description(void){
        stringstream ret;
        ret << weight << "斤猪肉";
        return ret.str();
    };
 
private: 
    int weight = 0;
};

//牛
class Cow { 
public:
    Cow(int weight){
        this->weight = weight;
    };
    // 参数此时定义为引用类型，更合适，避免拷贝
    Pork operator+(const Cow& cow){
        int tmp = (this->weight + cow.weight) * 2; 
        return Pork(tmp);
    }; //同类型进行运算，很频繁

    Pork operator+(const Goat& goat){
        int tmp = this->weight * 2 + goat.getWeight() * 3; 
        return Pork(tmp);
    }; //不同类型进行运算，比较少见 
private: 
    int weight = 0;
};

int main(){
    Cow c1(100);
    Cow c2(200);
    // 调用c1.operator+(c2);
    //相当于：Pork p = c1.operator+(c2); 
    Pork p = c1 + c2;
    cout << p.description() << endl;
    
    Goat g1(100);
    p = c1 + g1;
    cout << p.description() << endl;
    
    return 0;
}