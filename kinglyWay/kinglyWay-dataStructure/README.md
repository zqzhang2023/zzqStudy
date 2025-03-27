# 数据结构 王道2026学习笔记

# 第一章 绪论

## 1.1 数据结构的基本概念

### 概念和术语

- **数据**数据是描述客观事物的符号，是计算机中可以操作的对象，是能被计算机识别，并输入给计算机处理的符号集合。

- **数据元素**数据元素是数据的基本单位，在计算机中通常作为一个整体进行考虑和处理。

- **数据项**一个数据元素可以由若干个数据项组成，数据项是数据不可分割的最小单位。

- **数据对象**数据对象是性质相同的数据元素的集合，是数据的子集。

- **数据类型** 数据类型是一个值的集合和定义在此集合上的一组操作的总称。
  - 原子类型：其值不可再分的类型，如整型、浮点型、字符型等。
  - 结构类型：其值可以再分的类型，如整形数组、浮点型数组、结构体、链表、栈、队列等。
  - 抽象数据类型（Abstract Data Type, ADT）：抽象数据类型是指一个数学模型及定义在该模型上的一组操作。

### 数据结构三要素

1.逻辑结构：数据元素之间的逻辑关系。

2.存储结构：数据元素及其关系在计算机存储器中的表示。

3.数据运算：针对某种逻辑结构在相应存储结构上的操作。

**逻辑结构**

![alt text](0_images/1_1_数据的逻辑结构.png)

![alt text](0_images/1_2_数据的逻辑结构详解.png)

- 集合：集合结构中的数据元素除了同属于一个集合外，它们之间没有其他关系。
- 线性结构：线性结构中的数据元素之间存在一对一的关系。
- 树形结构：树形结构中的数据元素之间存在一对多的层次关系。
- 图状结构或网状结构：图状结构中的数据元https://xiaolincoding.com/素之间存在多对多的关系。
  
**存储结构**

- 顺序存储: 用一组连续的存储单元依次存储数据元素，数据元素之间的逻辑关系由元素的存储位置来表示。
- 链式存储: 用一组任意的存储单元存储数据元素，数据元素之间的逻辑关系用指针来表示。
- 索引存储: 在存储数据元素的同时，还建立附加的索引表，索引表中的每项称为索引项，索引项的一般形式是（关键字，地址）。
- 散列存储: 散列存储是根据元素的关键字直接计算出该元素的存储地址，因此散列存储不需要建立索引表。
  
**数据运算**
- 包括运算的定义和运算的实现两个部分。
- 数据运算的定义是针对逻辑的，指出运算的功能
- 数据运算的实现是针对存储的，指出运算的具体实现方法

## 1.2 算法

### 算法的概念

算法是解决特定问题求解步骤的描述，在计算机中表现为指令的有限序列，并且每条指令表示一个或多个操作。


算法具有以下五个特性(了解)：
- 有穷性：一个算法必须总在执行有穷步之后结束，且每一步都可在有穷时间内完成。
- 确定性：算法中每条指令必须有确切的含义，对于相同的输入只能得到相同的输出。  
- 可行性：算法的每一步操作都是可行的，即每一步都能够通过执行有限次数完成。
- 输入：一个算法有零个或多个输入，这些输入取自于某个特定的对象的集合。
- 输出：一个算法有一个或多个输出，这些输出是与输入有着某种特定关系的量。

算法达到的目标(了解)：

- 正确性：算法对于合法的输入，能够产生满足要求的输出，对非法输入能够做出相应处理。
- 可读性：算法应具有良好的可读性，以帮助人们理解。
- 健壮性：当输入数据不合法时，算法也能做出相关处理，而不是产生异常或莫名其妙的结果。
- 高效性：算法应尽量简短，算法的执行时间应尽可能短，算法所需的存储空间应尽量小。

### 算法效率的度量！！！

时间复杂度：算法的时间复杂度是指算法执行时间随问题规模n的变化而变化的函数，记作：T(n)=O(f(n))。其中f(n)是问题规模n的某个函数，O是数量级符号。

空间复杂度：算法的空间复杂度是指算法在计算机内执行时所需存储空间的度量，记作：S(n)=O(g(n))。其中g(n)是问题规模n的某个函数，O是数量级符号。


# 第二章 线性表

# 第三章 栈、队列和数组

## 栈

### 栈的定义和基本操作

只允许在一端进行插入或删除的线性表

![alt text](0_images/3_1_栈.png)

当有n个不同的元素入栈的时候，出栈排列有(1/n+1)/C^n_{2n}种

**共享栈**

一个数组两个栈用，一个用栈底一个用栈顶（好奇怪）

栈的实现（数组实现比较简单，这里采用链栈实现）

链栈的优点在于不存在上溢问题

```cpp
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
```

## 队列
### 队列的定义

先进先出的受限线性表

![alt text](0_images/3_2_队列.png)

### 顺序存储

这里有一个假溢出的概念

![alt text](0_images/3_3_队列的假溢出.png)

这个时候如果使用rear == front 判断栈满，就会出现假溢出现象

引入循环队列：

![alt text](0_images/3_4_循环队列.png)

还是无法判断队列满不满（rear == front 可能是空，也可能是满）

- 牺牲一个存储单元
  - (rear+1) % maxSize == front 满
  - rear == front  空
  - (rear - front + MaxSize) % MaxSize 队内元素
- 增加size属性
  - size == 0 空
  - size == maxSize 满
- 增加tag
  - 出队的时候 将 tag = 0, 入队的时候 将 tag = 1
  - tag == 0 && rear == front 空
  - tag == 1 && rear == front 满
  
### 循环队列的实现

```cpp
#include<iostream>
using namespace std;

#define MaxSize 100
enum operation_code {success,overflow,underflow};

template <typename T>
class Queue{
private:
    T data[MaxSize];
    int size;
    int front;
    int rear;
public:
    //构造初始化
    Queue(){
        front = 0;
        rear = 0;
        size = 0;
    }
    //判空
    bool isEmpty(){
        return size==0;
    }
    //判满
    bool isFull(){
        return size==MaxSize;
    }
    //入队
    operation_code push(T x){
        if (isFull()){
            return overflow;
        }
        data[rear]= x;
        rear = (rear + 1)%MaxSize;
        size++;
        return success;
    }
    //出队
    operation_code pop(T &x){
        if (isEmpty()){
            return underflow;
        }
        x = data[front];
        front = (front + 1)%MaxSize;
        size--;
        return success;
    }
};

int main() {
    Queue<int> q;
    int x;

    // 填满队列
    for (int i = 0; i < 100; ++i) {
        if (q.push(i) != success) {
            cout << "队列已满，无法入队元素 " << i << endl;
        }
    }

    // 测试满队列入队
    operation_code result = q.push(100);
    cout << "满队列入队结果: " << result << " (1 表示overflow)" << endl;

    // 清空队列
    while (q.pop(x) == success) {
        cout << x << " ";
    }
    cout << "\n队列已清空" << endl;

    // 测试空队列出队
    result = q.pop(x);
    cout << "空队列出队结果: " << result << " (2 表示underflow)" << endl;

    // 正常入队和出队测试
    q.push(10);
    q.push(20);
    q.pop(x);
    cout << "出队元素: " << x << endl;  // 应输出10
    q.pop(x);
    cout << "出队元素: " << x << endl;  // 应输出20

    return 0;
}
```


### 链式队列

```cpp
#include<iostream>
using namespace std;

enum operation_code {success,overflow,underflow};

template <typename T>
struct Node{
    T data;
    Node<T>* next;
};

template <typename T>
class Queue{
private:
    Node<T>* front;
    Node<T>* rear;
    int size;
public:
    Queue(){
        front = new Node<T>();
        front->next = nullptr;
        rear = front;
        size = 0;
    }

    //判空
    bool isEmpty(){
        // return rear == front;
        return size == 0;
    }

    //入队
    operation_code push(T x){
        Node<T> * newNode = new Node<T>();
        newNode->data = x;
        newNode->next = nullptr;
        rear->next = newNode;
        rear = newNode;
        size++;
        return success;
    }   

    //出队
    operation_code pop(T &x){
        if (isEmpty()){
            return underflow;
        }
        Node<T> *tmp = front->next;
        x = tmp->data;
        front->next = tmp->next;
        if (rear == tmp){
            rear = front;
        }
        delete tmp;
        size--;
        return success;      
    }

    ~Queue(){
        while (front != nullptr) {
            Node<T>* tmp = front;
            front = front->next;
            delete tmp;
        }
    }
};




int main(){
    Queue<int> q;
    int val;

    // 测试空队列出队
    cout << "空队列出队结果: " << q.pop(val) << " (应返回2 underflow)" << endl;

    // 入队测试
    q.push(10);
    q.push(20);
    q.push(30);

    // 出队测试
    q.pop(val);
    cout << "第一次出队元素: " << val << " (应输出10)" << endl;
    q.pop(val);
    cout << "第二次出队元素: " << val << " (应输出20)" << endl;

    // 再次入队
    q.push(40);
    q.pop(val);
    cout << "第三次出队元素: " << val << " (应输出30)" << endl;
    q.pop(val);
    cout << "第四次出队元素: " << val << " (应输出40)" << endl;

    // 测试空队列
    cout << "空队列出队结果: " << q.pop(val) << " (应返回2 underflow)" << endl;

    return 0;
}
```

### 双端队列（了解）

![alt text](0_images/3_5_双端口队列.png)

## 栈和队列的应用

看一看书，队列的遍历非常重要！！！

![alt text](0_images/3_6_队列在遍历之中的应用.png)

## 数组（了解）



