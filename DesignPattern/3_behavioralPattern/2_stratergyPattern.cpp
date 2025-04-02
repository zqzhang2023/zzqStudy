#include<iostream>
#include <memory>
using namespace std;

// 策略接口：支付方式
class PaymentStrategy{
public:
    virtual void pay(double amount) const = 0;
    virtual ~PaymentStrategy() = default;
};

// 具体策略：信用卡支付
class CreditCardPayment:public PaymentStrategy{
    void pay(double amount) const override{
        cout << "使用信用卡支付：" << amount << "元" <<endl;;
    }
};

// 具体策略：支付宝支付
class AlipayPayment:public PaymentStrategy{
public:
    void pay(double amount) const override{
        cout << "使用支付宝支付：" << amount << "元" <<endl;
    }
};

// 具体策略：微信支付
class WeChatPayment:public PaymentStrategy{
public:
    void pay(double amount) const override{
        cout << "使用微信支付：" << amount << "元" <<endl;
    }
};

// 上下文类：订单
class Order{
private:
    unique_ptr<PaymentStrategy> thisPaymentStrategy;
public:
    void setPaymentStrategy(unique_ptr<PaymentStrategy> strategy){
        thisPaymentStrategy = move(strategy);
    }

    void checkout(double amount){
        if (thisPaymentStrategy) {
            thisPaymentStrategy->pay(amount);
        } else {
            std::cout << "错误：未选择支付方式！\n";
        }
    }

};


int main(){

    Order order;

    // 使用信用卡支付
    order.setPaymentStrategy(make_unique<CreditCardPayment>());
    order.checkout(100.50);
    
    // 切换为支付宝支付
    order.setPaymentStrategy(std::make_unique<AlipayPayment>());
    order.checkout(200.0);
    
    // 切换为微信支付
    order.setPaymentStrategy(std::make_unique<WeChatPayment>());
    order.checkout(50.0);



    return 0;
}