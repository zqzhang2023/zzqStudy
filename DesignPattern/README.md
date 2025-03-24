# C++设计模式
**资料**https://blog.csdn.net/qq_52860170/article/details/142577985?ops_request_misc=%257B%2522request%255Fid%2522%253A%25228e11db211db1f2965a767de671477559%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=8e11db211db1f2965a767de671477559&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_positive~default-2-142577985-null-null.142^v102^pc_search_result_base4&utm_term=%E8%AE%BE%E8%AE%A1%E6%A8%A1%E5%BC%8F&spm=1018.2226.3001.4187

## 软件的设计原则

目的：为了提高软件系统的可维护性、可复用性，增强软件的可扩展性、灵活性

### 开闭原则（OCP）

软件实体对扩展是开放的，但对修改是关闭的，即在不修改一个软件实体的基础上去扩展其功能。

对扩展开放，对修改关闭

实现：接口和抽象类

### 里氏替换原则（LSP）

任何基类可以出现的地方，子类一定可以出现，通俗来说，子类可以扩展父类的功能，但不能改变父类原有的功能

### 依赖倒置原则(DIP)

高层模块不应该依赖于低层模块，两者都应该依赖于抽象；抽象不应该依赖于细节，细节应该依赖于抽象；对抽象编程，不对实现编程，降低了客户与实现模块间的耦合。

### 接口隔离原则(ISP)

客户端不应该被迫依赖于它不使用的方法；一个类对另一个类的依赖应该建立在最小的接口上。

### 单一职责原则(SRP)

一个类应该有且仅有一个职责

比如：手机：拍照、摄影、游戏、GPS

拆分成多个类:

拍摄职责：照相机、摄影机

游戏：PSP

GPS：专业GPS导航

## 高内聚、低耦合

## 创建型模式

### 简单工厂模式

只需要知道水果的名字则可得到相应的水果

![alt text](0_images/1_简单工厂模式.png)

![alt text](0_images/2_简单工厂模式_角色.png)

角色：

- Factory：工厂角色

- Product：抽象产品角色

- ConcreteProduct：具体产品角色


```cpp
#include<iostream>
using namespace std;

// 抽象产品类
class Shape{
public:
    virtual void draw() const = 0;
    virtual ~Shape(){};
};

// 具体的产品：圆形
class Circle: public Shape{
public:
    void draw() const override{
        cout<<"绘制圆形"<<endl;
    }

};

// 具体的产品：矩形
class Rectangle : public Shape{
public:
    void draw() const override{
        cout<<"绘制矩形"<<endl;
    }
};

// 工厂类，静态的创建函数
class ShapeFactory {
public:
    static Shape* createShape(const string& type){
        if (type == "Circle") {
            return new Circle();
        }else if(type == "Rectangle"){
            return new Rectangle();
        }
        throw invalid_argument("无效的形状类型！");        
    }

};


int main(){

    try{
        Shape* circle = ShapeFactory::createShape("Circle");
        Shape* rectangle = ShapeFactory::createShape("Rectangle");

        circle->draw();      // 输出：绘制圆形
        rectangle->draw();   // 输出：绘制矩形

        delete circle;
        delete rectangle;

        Shape* unknown = ShapeFactory::createShape("Triangle");

    }catch(const exception& e){
        cout << e.what() << endl;
    }
    
    return 0;
}
```

简单工厂模式虽然很好用，但是其违背了 **开闭原则** 当需要添加一个产品的时候，需要修改createShape部分的代码，因此便引入了工厂模式。

PPT之中一个例子
![alt text](0_images/3_简单工厂模式.png)


### 工厂模式

![alt text](0_images/4_工厂模式.png)

通过 将对象的实例化推迟到子类 来实现：每个具体工厂类只需要关注自己负责的产品创建，系统的扩展通过新增工厂-产品对实现，而不是修改现有代码。这种设计更符合面向对象的设计原则，特别适合需要支持多平台、多系列产品等场景。

- 抽象工厂：提供了创建产品的接口，调用者通过它访问具体工厂的工厂方法来创建产品

- 具体工厂：主要是实现抽象工厂中的抽象方法，完成具体产品的创建

- 抽象产品：定义了产品的规范，描述了产品的主要特性和功能

- 具体产品：实现了抽象产品角色所定义的接口，由具体工厂来创建，它同具体工厂之间一一对应

```cpp
#include<iostream>
using namespace std;

// 抽象产品类
class Shape{
public:
    virtual void draw() const = 0;
    virtual ~Shape(){};
};

// 具体的产品：圆形
class Circle: public Shape{
public:
    void draw() const override{
        cout<<"绘制圆形"<<endl;
    }

};

// 具体的产品：矩形
class Rectangle : public Shape{
public:
    void draw() const override{
        cout<<"绘制矩形"<<endl;
    }
};

// 添加产品： 三角形
class Triangle : public Shape{
public:
    void draw() const override{
        cout<<"绘制三角形"<<endl;
    }
};

// 抽象工厂类
class ShapeFactory {
public:
    virtual Shape* createShape() const = 0;
    virtual ~ShapeFactory(){};
};

// 圆形工厂
class CircleFactory: public ShapeFactory{
public:
    Shape* createShape() const override{
        return new Circle();
    }
};

// 矩形工厂
class RectangleFactory: public ShapeFactory{
public:
    Shape* createShape() const override{
        return new Rectangle();
    }
};

//新增 三角形工厂
class TriangleFactory: public ShapeFactory{
public:
    Shape* createShape() const override{
        return new Triangle();
    }
};



int main(){

    ShapeFactory* circleFactory = new CircleFactory();
    Shape* circle = circleFactory->createShape();
    
    ShapeFactory* rectangleFactory = new RectangleFactory();
    Shape* rectangle = rectangleFactory->createShape();

    circle->draw();      // 输出：绘制圆形
    rectangle->draw();   // 输出：绘制矩形

    //新增：
    ShapeFactory* triangleFactory = new TriangleFactory();
    Shape* triangle = triangleFactory->createShape();
    triangle->draw();


    delete circle;
    delete rectangle;
    delete circleFactory;
    delete rectangleFactory;

    delete triangle;
    delete triangleFactory;
    

    return 0;
}
```

### 抽象工程模式

提供一个创建一系列相关或相互依赖对象的接口，而无需指定他们具体的类

当系统所提供的工厂所需生产的具体产品并不是一个简单的对象，而是多个位于不同产品等级结构中属于不同类型的具体产品时需要使用抽象工厂模式

抽象工厂模式与工厂方法模式最大的区别在于，工厂方法模式针对的是一个产品等级结构，而抽象工厂模式则需要面对多个产品等级结构。

- 抽象工厂：提供了创建产品的接口，包含多个创建产品的方法，可以创建多个不同等级的产品

- 具体工厂：实现了抽象工厂中的多个抽象方法，完成具体产品的创建

- 抽象产品：定义了产品的规范，描述了产品的主要特征和功能，抽象工厂模式有多个抽象产品

- 具体产品：实现了抽象产品所定义的接口，由具体工厂来创建，它具体工厂是多对一关系


![alt text](0_images/5_抽象工厂模式.png)

```cpp
#include<iostream>
#include <memory>
using namespace std;

// ------------------------- 抽象产品接口 -------------------------
// 按钮抽象类
class Button{
public:
    virtual void render() const = 0;
    virtual ~Button() = default;
};

// 文本框抽象类
class TextBox{
public:
    virtual void input(const string& text) const = 0;
    virtual ~TextBox() = default;
};

// 开关抽象类
class Switch{
public:
    virtual void toggle() const = 0;
    virtual ~Switch() = default;
};

// ------------------------- 具体产品实现 -------------------------
// 现代风格组件
class ModernButton: public Button{
public:
    void render() const override {
        cout << "渲染现代风格按钮 [▣ Minimal]" << endl;
    }
};

class ModernTextBox : public TextBox {
public:
    void input(const string& text) const override {
        cout << "现代风格文本框输入: \"" << text << "\" (无边框)" << endl;
    }
};

class ModernSwitch : public Switch {
public:
    void toggle() const override {
        cout << "切换现代风格开关 ●━━━━●" << endl;
    }
};

// 复古风格组件
class RetroButton : public Button {
public:
    void render() const override {
        cout << "渲染复古风格按钮 [▣ Beveled]" << endl;
    }
};

class RetroTextBox : public TextBox {
public:
    void input(const string& text) const override {
        cout << "复古风格文本框输入: \"" << text << "\" (像素字体)" << endl;
    }
};

class RetroSwitch : public Switch {
public:
    void toggle() const override {
        cout << "切换复古开关 ◈━━━◈" << endl;
    }
};

// ------------------------- 抽象工厂接口 -------------------------
class UIFactory{
public:
    virtual unique_ptr<Button> createButton() = 0;
    virtual unique_ptr<TextBox> createTextBox() = 0;
    virtual unique_ptr<Switch> createSwitch() = 0;
    virtual ~UIFactory() = default;
};

// ------------------------- 具体工厂实现 -------------------------
class ModernUIFactory : public UIFactory {
public:
    unique_ptr<Button> createButton() override {
        return make_unique<ModernButton>();
    }

    unique_ptr<TextBox> createTextBox() override {
        return make_unique<ModernTextBox>();
    }

    unique_ptr<Switch> createSwitch() override {
        return make_unique<ModernSwitch>();
    }
};

class RetroUIFactory : public UIFactory {
public:
    unique_ptr<Button> createButton() override {
        return make_unique<RetroButton>();
    }

    unique_ptr<TextBox> createTextBox() override {
        return make_unique<RetroTextBox>();
    }

    unique_ptr<Switch> createSwitch() override {
        return make_unique<RetroSwitch>();
    }
};

// ------------------------- 客户端代码 -------------------------
void createUI(const unique_ptr<UIFactory>& factory) {
    auto button = factory->createButton();
    auto textbox = factory->createTextBox();
    auto switchCtrl = factory->createSwitch();

    button->render();
    textbox->input("Hello World");
    switchCtrl->toggle();
}


int main(){

    cout << "=== 现代风格 UI ===" << endl;
    createUI(make_unique<ModernUIFactory>());

    cout << "\n=== 复古风格 UI ===" << endl;
    createUI(make_unique<RetroUIFactory>());


    return 0;
}
```

三种工厂模式对比表

| 特性       | 简单工厂模式           | 工厂方法模式               | 抽象工厂模式                 |
|------------|------------------------|----------------------------|------------------------------|
| 创建目标   | 单一产品               | 单一产品                   | 产品家族（多个关联产品）      |
| 扩展维度   | 垂直扩展（产品类型）   | 垂直扩展（产品类型）       | 水平扩展（产品家族）          |
| 开闭原则   | 违反                   | 支持产品扩展               | 支持产品族扩展               |
| 工厂类数量 | 1个全能工厂            | N个工厂对应N个产品         | N个工厂对应N个产品族         |
| 适用场景   | 简单对象创建           | 单一产品扩展               | 多系列关联对象创建            |


### 单例模式

单例模式确保一个类只有一个实例，并提供一个全局访问点。

![alt text](0_images/6_单例模式.png)

```cpp
#include <iostream>
using namespace std;

class Singleton {
private:
    Singleton() = default; // 私有构造函数
    ~Singleton() = default; // 私有析构函数（可选）
    
    // 删除拷贝和赋值操作
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton& getInstance() {
        static Singleton instance; // 线程安全且自动析构
        return instance;
    }

    void testFunction() {
        cout << "testFunction" << endl;
    }
};

int main() {
    Singleton::getInstance().testFunction();
    return 0;
}
```