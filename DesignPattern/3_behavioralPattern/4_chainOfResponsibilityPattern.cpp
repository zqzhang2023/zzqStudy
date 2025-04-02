#include<iostream>
using namespace std;

// 请求类（采购请求）
class PurchaseRequest{
public:
    int type;  // 请求类型
    int id;     // 请求ID
    double amount; // 金额
    
    PurchaseRequest(int t, int i, double a)
        : type(t), id(i), amount(a) {}
};

//抽象处理者
class Approver{
protected:
    Approver * successor; // 下一级处理者
    string name;         // 处理者名称
public:
    Approver(string n) : name(n), successor(nullptr) {}
    void setSuccessor(Approver* s) {
        successor = s;
    }  
    virtual void processRequest(PurchaseRequest* request) = 0;
};

// 具体处理者：经理（可审批5000元以下）
class Manager :public Approver{
public:
    Manager(string n) : Approver(n) {};

    void processRequest(PurchaseRequest* request){
        if(request->amount<5000){
            cout << name << "经理审批采购单#"
                 << request->id << "，金额："
                 << request->amount << "元" << endl;
        }else if (successor != nullptr) {
            successor->processRequest(request);
        }
    }
};

// 具体处理者：总监（可审批10000元以下）
class Director : public Approver {
public:
    Director(string n) : Approver(n) {}
    
    void processRequest(PurchaseRequest* request) override {
        if (request->amount < 10000) {
            cout << name << "总监审批采购单#"
                 << request->id << "，金额："
                 << request->amount << "元" << endl;
        } else if (successor != nullptr) {
            successor->processRequest(request);
        }
    }
};


// 具体处理者：CEO（可审批任意金额）
class CEO : public Approver {
public:
    CEO(string n) : Approver(n) {}
    
    void processRequest(PurchaseRequest* request) override {
        cout << name << "CEO审批采购单#"
             << request->id << "，金额："
             << request->amount << "元" << endl;
    }
};

int main(){

    // 创建处理者
    Manager manager("张");
    Director director("王");
    CEO ceo("李");

    // 设置责任链
    manager.setSuccessor(&director);
    director.setSuccessor(&ceo);

    // 创建请求
    PurchaseRequest req1(1, 1001, 4500);
    PurchaseRequest req2(2, 1002, 8000);
    PurchaseRequest req3(3, 1003, 150000);

    // 从链首开始处理请求
    manager.processRequest(&req1); // 经理处理
    manager.processRequest(&req2); // 总监处理
    manager.processRequest(&req3); // CEO处理

    return 0;
}