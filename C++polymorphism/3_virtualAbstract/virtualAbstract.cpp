#include <iostream>
#include <string>
 
using namespace std;
 
class Shape {
public:
    Shape(const string& color = "white") { this->color = color; }
    virtual float area() = 0; //不用做具体的实现
    string getColor() { return color; }
private:
    string color;
};
 
class Circle : public Shape {
public:
    Circle(float radius = 0, const string& color="White"):Shape(color), r(radius){}
    float area();
private:
    float r; //半径
};
 
float Circle::area() {
    return 3.14 * r * r;
}
 
int main() {
//使用抽象类创建对象非法！
//Shape s;
    Circle c1(10);
    cout << c1.area() << endl;
    Shape* p = &c1;
    cout << p->area() << endl;

    return 0;
}