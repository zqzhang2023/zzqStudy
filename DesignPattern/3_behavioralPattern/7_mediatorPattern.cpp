#include<iostream>
#include<string>
#include <vector>
using namespace std;

// 前置声明
class Colleague;

// 抽象中介者
class Mediator {
public:
    virtual void sendMessage(const string& message,const Colleague* sender) = 0;
    virtual void addColleague(Colleague* colleague) = 0;
    virtual ~Mediator() = default;
};

// 抽象同事类
class Colleague{
protected:
    Mediator* mediator_;
    string name_;
public:
    Colleague(Mediator* mediator, const string& name)
        : mediator_(mediator), name_(name) {}

    virtual void send(const string& message) = 0;
    virtual void receive(const string& message) = 0;
    virtual ~Colleague() = default;
    string getName() const { return name_; }
};

// 具体中介者（聊天室）
class ChatRoomMediator:public Mediator{
private:
    vector<Colleague*> colleagues_;
public:
    void addColleague(Colleague* colleague) override {
        colleagues_.push_back(colleague);
    }

    void sendMessage(const string& message, const Colleague* sender) override {
        for (auto colleague : colleagues_) {
            // 不将消息发送给发送者自己
            if (colleague != sender) {
                colleague->receive(message);
            }
        }
    }
};

// 具体同事类（用户）
class User: public Colleague{
public:
    User(Mediator* mediator, const string& name)
    : Colleague(mediator, name) {
        mediator_->addColleague(this);
    }

    void send(const string& message) override {
        cout << name_ << " 发送消息: " << message << endl;
        mediator_->sendMessage(message, this);
    }

    void receive(const string& message) override {
    cout << name_ << " 收到消息: " << message << endl;
    }
};

// 使用示例
int main() {
    ChatRoomMediator chatRoom;

    User alice(&chatRoom, "Alice");
    User bob(&chatRoom, "Bob");
    User charlie(&chatRoom, "Charlie");

    alice.send("大家好！");
    cout << endl;
    bob.send("今天天气不错！");
    cout << endl;
    charlie.send("有人想去喝咖啡吗？");

    return 0;
}