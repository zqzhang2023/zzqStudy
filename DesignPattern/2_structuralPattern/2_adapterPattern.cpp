#include<iostream>
using namespace std;

// 目标接口（Target Interface） - 客户端期望的充电器规格
class Charger{
public:
    virtual void charge() const = 0;
    virtual ~Charger() = default;
};


// 被适配者（Adaptee） - 已存在的欧洲插头实现
class EuroPlug{
public:
    void specificCharge() const {
        cout << "⚡ 使用欧洲插头充电 (220V)" << endl;
    }
};

// 适配器（Adapter） - 将欧洲插头适配到标准充电器接口
class EuroChargerAdapter : public Charger{
private:
    EuroPlug* euroPlug_;  // 持有被适配对象的指针
public:
    EuroChargerAdapter(EuroPlug* plug){
        this->euroPlug_ = plug;
    }

    void charge() const override {
        if(euroPlug_) {
            //使用转接口之后，就可以用欧洲的插头了
            cout << "🔌 使用电源适配器转换" << endl;
            euroPlug_->specificCharge();
        }
    }

};

// 客户端代码
int main() {
    // 创建被适配的欧洲插头
    EuroPlug euroPlug;
    
    // 创建适配器并将欧洲插头接入
    Charger* charger = new EuroChargerAdapter(&euroPlug);
    
    // 使用标准接口充电
    charger->charge();
    
    delete charger;
    return 0;
}