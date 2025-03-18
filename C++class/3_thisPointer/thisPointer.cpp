#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Box {  
    public:  
        double width;  
        double height;  
      
        // 构造函数  
        Box(double width, double height) {
            //区分局部变量和成员变量
            this->width = width;
            this->height = height;
        }  
      
        // 成员函数，用于计算盒子的体积  
        double getVolume() {  
            return width * height;  
        }  
      
        // 成员函数，使用 this 指针返回指向当前对象的指针  
        Box* getBiggerBox(double extraWidth, double extraHeight) {  
            this->width += extraWidth; // 使用 this 指针访问成员变量  
            this->height += extraHeight;  
            return this; // 返回当前对象的指针  
        }  
};  

void processObject(Box* obj){  
    cout<<obj->getVolume()<<endl;
}  
      
int main() {  
    Box myBox(3.0, 4.0);  
    cout << "Original Volume: " << myBox.getVolume() << endl;  
      
        // 使用 getBiggerBox 成员函数改变 myBox 的尺寸，并返回 myBox 的引用  链式调用
    Box* biggerBox = myBox.getBiggerBox(1.0, 1.0)->getBiggerBox(1.0,1.0);  
      
        // 输出改变后的体积  
    cout << "Bigger Volume: " << biggerBox->getVolume() << endl;  

    processObject(myBox.getBiggerBox(1.0, 1.0));
    
      
    return 0;  
}