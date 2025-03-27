#include<iostream>
using namespace std;

enum operation_code {success,overflow,underflow};

template <typename T>
struct Node{
    T data;
    Node<T>* next;
};

template <typename T>
class LinkStack{
private:
    Node<T> *top;
    int stackSize;

public:
    LinkStack(){
        top = new Node<T>();
        top->next = nullptr;
        stackSize = 0;
    };

    //判空
    bool isEmpty(){
        return stackSize==0;
    }

    //入栈
    operation_code push(T data){
        Node<T>* newNode = new Node<T>();
        newNode->data = data;
        newNode->next = top->next;
        top->next = newNode;
        stackSize++;
        return success;
    }

    //读栈顶元素
    operation_code getTop(T &x){
        if (!top->next){
            return underflow;
        }
        x = top->next->data;
        return success;
    }

    //出栈
    operation_code pop(T &x){
        if (!top->next){
            return underflow;
        }
        Node<T>* popNode = top->next;
        x = popNode->data;
        top->next = popNode->next;
        delete popNode;
        stackSize--;
        return success;
    }

    ~LinkStack(){
        T temp;
        while(!isEmpty()){
            pop(temp);
        }
        delete top;
        top = nullptr;
    }
};


int main(){
    LinkStack<int> stack;

    // 测试空栈弹出
    int x;
    if (stack.pop(x) == underflow) {
        cout << "1. 空栈弹出失败测试通过" << endl;
    }

    // 测试入栈和栈顶
    stack.push(10);
    stack.getTop(x);
    cout << "2. 当前栈顶元素（应为10）: " << x << endl;

    stack.push(20);
    stack.push(30);

    // 连续弹出测试
    stack.pop(x);
    cout << "3. 弹出元素（应为30）: " << x << endl;
    stack.pop(x);
    cout << "4. 弹出元素（应为20）: " << x << endl;

    // 测试栈顶更新
    stack.getTop(x);
    cout << "5. 当前栈顶元素（应为10）: " << x << endl;

    // 清空栈测试
    stack.pop(x);
    cout << "6. 弹出元素（应为10）: " << x << endl;

    if (stack.isEmpty()) {
        cout << "7. 栈已空测试通过" << endl;
    }

    // 再次测试空栈操作
    if (stack.getTop(x) == underflow) {
        cout << "8. 空栈获取栈顶测试通过" << endl;
    }

    return 0;
}
