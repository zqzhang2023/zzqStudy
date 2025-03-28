#include<iostream>
#include<memory>
#include<string>
using namespace std;

class DrawAPI{
public:
    virtual void draw(const string& shape) = 0;
    virtual ~DrawAPI() = default;
};

// 具体实现：使用OpenGL绘图
class OpenGLAPI:public DrawAPI{
public:
    void draw(const string& shape) override{
        cout << "OpenGL绘制: "<<shape<< endl;
    }
};

// 具体实现：使用DirectX绘图
class DirectXAPI: public DrawAPI{
public:
void draw(const string& shape) override{
    cout << "DirectX绘制: "<<shape<< endl;
}
};

// 抽象部分：图形基类
class Shape{
protected:
    unique_ptr<DrawAPI> drawAPI;
public:
    Shape(DrawAPI * api):drawAPI(api){};
    virtual void draw() = 0;
    virtual ~Shape() = default;
};

//圆形
class Circle:public Shape{
private:
    string myShape;
public:
    Circle(string shape,DrawAPI * api):Shape(api),myShape(shape){};
    void draw() override{
        drawAPI->draw(this->myShape);
    }
};

//矩形
class Rectangle:public Shape{
private:
    string myShape;
public:
    Rectangle(string shape,DrawAPI * api):Shape(api),myShape(shape){};
    void draw() override{
        drawAPI->draw(this->myShape);
    }
};

int main(){

    // 使用OpenGL绘制圆形
    unique_ptr<Shape> circle = make_unique<Circle>("圆形", new OpenGLAPI());
    // 使用DirectX绘制矩形
    unique_ptr<Shape> rect = make_unique<Rectangle>("矩形", new DirectXAPI());
    
    circle->draw();
    rect->draw();

    // 测试其他组合
    Shape* openglRect = new Rectangle("矩形", new OpenGLAPI());
    Shape* directxCircle = new Circle("圆形", new DirectXAPI());
    
    openglRect->draw();
    directxCircle->draw();
    
    delete openglRect;
    delete directxCircle;

    return 0;
}