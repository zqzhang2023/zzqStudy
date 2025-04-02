#include <iostream>
#include <memory>
using namespace std;

// 前向声明
class ElevatorContext;

// 抽象状态接口
class ElevatorState {
public:
    virtual void openDoors(ElevatorContext* context) = 0;
    virtual void closeDoors(ElevatorContext* context) = 0;
    virtual void move(ElevatorContext* context) = 0;
    virtual void stop(ElevatorContext* context) = 0;
    virtual ~ElevatorState() = default;
};

// 提前声明具体状态类（关键修改点）
class DoorsOpenState;
class DoorsClosedState;
class MovingState;

// 上下文类
class ElevatorContext {
private:
    unique_ptr<ElevatorState> currentState;

public:
    ElevatorContext(unique_ptr<ElevatorState> state);
    
    // 委托方法给当前状态
    void requestOpenDoors();
    void requestCloseDoors();
    void requestMove();
    void requestStop();

    // 状态转移
    void changeState(unique_ptr<ElevatorState> state);
    void displayState(const string& stateName);
};

// 具体状态类实现
class DoorsOpenState : public ElevatorState {
public:
    void openDoors(ElevatorContext* context) override;
    void closeDoors(ElevatorContext* context) override;
    void move(ElevatorContext* context) override;
    void stop(ElevatorContext* context) override;
};

class DoorsClosedState : public ElevatorState {
public:
    void openDoors(ElevatorContext* context) override;
    void closeDoors(ElevatorContext* context) override;
    void move(ElevatorContext* context) override;
    void stop(ElevatorContext* context) override;
};

class MovingState : public ElevatorState {
public:
    void openDoors(ElevatorContext* context) override;
    void closeDoors(ElevatorContext* context) override;
    void move(ElevatorContext* context) override;
    void stop(ElevatorContext* context) override;
};

// 上下文类方法实现
ElevatorContext::ElevatorContext(unique_ptr<ElevatorState> state)
    : currentState(move(state)) {}

void ElevatorContext::requestOpenDoors() { currentState->openDoors(this); }
void ElevatorContext::requestCloseDoors() { currentState->closeDoors(this); }
void ElevatorContext::requestMove() { currentState->move(this); }
void ElevatorContext::requestStop() { currentState->stop(this); }

void ElevatorContext::changeState(unique_ptr<ElevatorState> state) {
    currentState = move(state);
}

void ElevatorContext::displayState(const string& stateName) {
    cout << "Elevator is now in [" << stateName << "] state\n";
}

// 具体状态类方法实现
void DoorsOpenState::openDoors(ElevatorContext* context) {
    cout << "Doors are already open\n";
}

void DoorsOpenState::closeDoors(ElevatorContext* context) {
    cout << "Closing doors...\n";
    context->changeState(make_unique<DoorsClosedState>());
    context->displayState("Doors Closed");
}

void DoorsOpenState::move(ElevatorContext* context) {
    cout << "Cannot move while doors are open\n";
}

void DoorsOpenState::stop(ElevatorContext* context) {
    cout << "Already stopped with open doors\n";
}

void DoorsClosedState::openDoors(ElevatorContext* context) {
    cout << "Opening doors...\n";
    context->changeState(make_unique<DoorsOpenState>());
    context->displayState("Doors Open");
}

void DoorsClosedState::closeDoors(ElevatorContext* context) {
    cout << "Doors are already closed\n";
}

void DoorsClosedState::move(ElevatorContext* context) {
    cout << "Starting movement...\n";
    context->changeState(make_unique<MovingState>());
    context->displayState("Moving");
}

void DoorsClosedState::stop(ElevatorContext* context) {
    cout << "Already stopped with closed doors\n";
}

void MovingState::openDoors(ElevatorContext* context) {
    cout << "Cannot open doors while moving\n";
}

void MovingState::closeDoors(ElevatorContext* context) {
    cout << "Doors are already closed\n";
}

void MovingState::move(ElevatorContext* context) {
    cout << "Already moving\n";
}

void MovingState::stop(ElevatorContext* context) {
    cout << "Stopping...\n";
    context->changeState(make_unique<DoorsClosedState>());
    context->displayState("Doors Closed");
}

int main() {
    ElevatorContext elevator(make_unique<DoorsOpenState>());
    
    elevator.requestCloseDoors(); // 关门并进入门已关闭状态
    elevator.requestMove();       // 开始移动
    elevator.requestOpenDoors();  // 尝试开门（应失败）
    elevator.requestStop();       // 停止并回到门已关闭状态
    elevator.requestOpenDoors();  // 开门回到初始状态

    return 0;
}