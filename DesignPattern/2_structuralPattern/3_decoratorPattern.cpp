#include<iostream>
#include<string>
using namespace std;

// 抽象组件
class Beverage {
public:
    virtual double cost() const = 0;
    virtual string description() const = 0;
    virtual ~Beverage() = default;
};

// 具体组件
class Espresso: public Beverage {
public:
    double cost() const override {
        return 1.99;
    }
    string description() const override{
        return "Espresso";
    }
};

// 抽象装饰者
class CondimentDecorator : public Beverage {
protected:
    Beverage* beverage;  // 被装饰的对象
public:
    CondimentDecorator(Beverage* b) : beverage(b) {};
    virtual ~CondimentDecorator(){
        delete beverage;
    }
};

// 具体装饰者：摩卡
class Mocha : public CondimentDecorator {
public:
    Mocha(Beverage* b) : CondimentDecorator(b) {}
    double cost() const override {
        return beverage->cost() + 0.20;
    }
    string description() const override{
        return beverage->description() + ", Mocha";
    }
};


// 具体装饰者：牛奶
class Milk : public CondimentDecorator {
public:
    Milk(Beverage* b) : CondimentDecorator(b) {}
    double cost() const override {
        return beverage->cost() + 0.50;
    }
    string description() const override {
        return beverage->description() + ", Milk";
    }
};

int main(){
    // 创建基础饮料
    Beverage* espresso = new Espresso();
    cout << espresso->description() 
              << " $" << espresso->cost() << endl;

    // 用装饰者添加配料
    Beverage* espressoWithMocha = new Mocha(espresso);
    Beverage* espressoWithMochaMilk = new Milk(espressoWithMocha);

    cout << espressoWithMochaMilk->description() 
              << " $" << espressoWithMochaMilk->cost() << endl;

    // 注意：装饰者会负责删除被装饰对象
    delete espressoWithMochaMilk;
}