#include<iostream>
using namespace std;

// 接收者接口（家电基类）
class Appliance {
public:
    virtual void on() = 0;
    virtual void off() = 0;
    virtual ~Appliance() = default;
};

// 具体接收者：电灯
class Light:public Appliance{
public:
    void on() override{
        cout<<"电灯打开了"<<endl;
    }
    void off() override{
        cout<<"电灯关闭了"<<endl;
    }
};

// 具体接收者：风扇
class Fan:public Appliance{
public:
    void on() override{
        cout<<"风扇打开了"<<endl;
    }
    void off() override{
        cout<<"风扇关闭了"<<endl;
    }
};

// 命令接口
class Command{
public:
    virtual void execute() = 0; 
    virtual void undo() = 0;
    virtual ~Command() = default;
};

// 具体命令：打开命令
class TurnOnCommand:public Command{
private:
    Appliance* appliance;
public:
    TurnOnCommand(Appliance* app) : appliance(app) {}
    void execute() override { appliance->on(); }
    void undo() override { appliance->off(); }
};

// 具体命令：关闭命令
class TurnOffCommand : public Command {
    Appliance* appliance;
public:
    TurnOffCommand(Appliance* app) : appliance(app) {}
    void execute() override { appliance->off(); }
    void undo() override { appliance->on(); }
};

// 调用者（遥控器按钮）
class RemoteControl{
private:
    Command * command;
    Command* lastCommand;
public:
    void setCommand(Command* cmd) {
        command = cmd;
    }

    void pressButton() {
        command->execute();
        lastCommand = command;
    }

    void pressUndo() {
        if(lastCommand) {
            lastCommand->undo();
            lastCommand = nullptr;
        }
    }

};

int main(){
// 创建家电设备
    Light livingRoomLight;
    Fan bathroomFan;

    // 创建命令
    TurnOnCommand lightOn(&livingRoomLight);
    TurnOffCommand fanOff(&bathroomFan);

    // 设置遥控器
    RemoteControl remote;
    
    // 测试电灯控制
    remote.setCommand(&lightOn);
    remote.pressButton();  // 开灯
    remote.pressUndo();    // 关灯

    // 测试风扇控制
    remote.setCommand(&fanOff);
    remote.pressButton();  // 关风扇
    remote.pressUndo();    // 开风扇

    return 0;
}