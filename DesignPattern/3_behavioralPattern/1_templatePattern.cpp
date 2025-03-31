#include<iostream>
using namespace std;

class Beverage{
public:
    virtual void prepareBeverage() final {
        boilWater();
        brew();
        pourInCup();
        if (customerWantsCondiments()) { // 钩子方法
            addCondiments();
        }
    }
protected:
    // 具体方法（已实现）
    void boilWater() {
        cout << "Boiling water" << endl;
    }

    void pourInCup() {
        cout << "Pouring into cup" << endl;
    }

    // 抽象方法（需要子类实现）
    virtual void brew() = 0;
    virtual void addCondiments() = 0;

    // 钩子方法（可选重写）
    virtual bool customerWantsCondiments() {
        return true; // 默认添加调料
    }
};

// 具体子类：咖啡
class Coffee:public Beverage{
protected:
    void brew() override {
        std::cout << "Brewing coffee grounds" << std::endl;
    }

    void addCondiments() override {
        std::cout << "Adding sugar and milk" << std::endl;
    }

    // 重写钩子方法
    bool customerWantsCondiments() override {
        char choice;
        std::cout << "Would you like milk and sugar with your coffee? (y/n) ";
        std::cin >> choice;
        return choice == 'y' || choice == 'Y';
    }
};

// 具体子类：茶
class Tea : public Beverage {
protected:
    void brew() override {
        std::cout << "Steeping the tea" << std::endl;
    }

    void addCondiments() override {
        std::cout << "Adding lemon" << std::endl;
    }

    // 不重写钩子方法，保持默认添加调料
};


// 使用示例
int main() {
    std::cout << "Making coffee:" << std::endl;
    Coffee coffee;
    coffee.prepareBeverage();

    std::cout << "\nMaking tea:" << std::endl;
    Tea tea;
    tea.prepareBeverage();

    return 0;
}