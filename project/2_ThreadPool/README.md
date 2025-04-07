# 线程池项目

## 基本概念

线程池的概念：

线程池是一种用于优化线程管理和任务调度的并发编程机制，其核心在于复用线程、控制资源使用，并提升系统性能。

说白了就是，为了防止这么构造线程的浪费太多的时间，先把线程创建好，放在那等着，有任务提交的话就直接处理，省去了构造线程的过程

那么我们就可以想到一个线程池的最基础的构造

变量：
- 一个vector 用来存储构造好的线程
- 一个queue 用来存储task (利用先进先出的思想，先来先服务 嘛 )
- mutex  有了queue自然得考虑到互斥的问题
- condition_variable 条件变量 同mutex理
- flag   停止标识

函数：
- 首先得有初始化函数，用来构造线程 (此处可以使用类的构造函数)
- 其次得有线程的释放join 的函数  （此处可以使用类的析构函数）
- 还有得有添加任务的函数

## ThreadPool声明

通过前面描述来声明一下CLASS

```cpp
class ThreadPool{
private:
    std::vector<std::thread> workers;        // 工作线程集合
    std::queue<std::function<void()>> tasks; // 任务队列（存储无参void函数）
    std::mutex queue_mutex;                  // 保护任务队列的互斥锁
    std::condition_variable condition;       // 线程间通信的条件变量
    bool stop;                               // 停止标志
public:
    ThreadPool(size_t); // 构造函数，参数为线程数量
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>; // 任务入队方法
    ~ThreadPool(); // 析构函数
};
```

这里其他的都比较好理解，主要是enqueue函数难以理解，这个先放一放，后面再详细解释

## ThreadPool构造函数，初始化函数的实现

这个其实没啥说的，很常规的东西

```cpp
inline ThreadPool::ThreadPool(size_t threadsNum):stop(false){
    for(size_t i=0; i<threadsNum; i++){
        workers.emplace_back( // 创建线程并绑定到Lambda
            [this]{
                for(;;){      // 无限循环，直到线程池停止
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,[this]{
                            return this->stop||!this->tasks.empty();
                        });// 等待任务或停止信号（如果任务列表为空或者是stop为true的话，就会一直等待）

                        if(this->stop && this->tasks.empty()){
                           return;   // 停止且无任务时退出线程
                        }
                        task = std::move(this->tasks.front());  //取任务
                        this->tasks.pop();
                    } //这里会自动释放锁
                    task(); //执行任务
                }
            }
        );
    }
}
```

## ~ThreadPool析构函数，函数的释放

emmm 这个也常规

```cpp
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        this->stop = true;  // 设置停止标志
    }
    condition.notify_all(); // 唤醒所有线程
    for(std::thread& worker:this->workers){
        worker.join();      // 等待所有线程结束
    }
}

```

## enqueue 添加任务功能的实现

这里就是最为复杂的地方，本来也是挺常规的，正常的传入（无参void函数）就行了，但是确实会出现问题，这样的话就只能处理 无参void函数 这一种类型了，这显然很有局限性。因此便有了这样一种书写方式

基本思想：将一个带参数的有返回值的函数，转化一下，转化为 无参void函数 就行了

先把代码发放上来，稍后一句一句分析

```cpp
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{   
    // 获取 F 用 Args... 调用后的返回类型。
    using return_type = typename std::result_of<F(Args...)>::type;  //若 F 是 int foo(double), Args 是 double，则 return_type 是 int。
    // 将用户任务转换为 return_type() 签名（无参数，返回 return_type）
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );

    // 从 packaged_task 中提取 future，用户可通过 res.get() 获取最终结果。
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(this->stop){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        this->tasks.emplace([task]() { (*task)(); });  // 将任务包装为 void() 类型
    }
    // 唤醒一个正在等待的 worker 线程来处理新任务。
    condition.notify_one();

    return res;
}
```

首先看一下函数的声明

```cpp
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
```

定义的是一个模板，template <class F, class... Args>  两个参数
- class F：接受任意类型的可调用对象（函数、Lambda、函数对象等）。
- class... Args：接受任意数量和类型的参数包。

函数参数
- F&& f：万能引用（不是右值引用！），可以绑定到任意类型的可调用对象（左值或右值）。
- Args&&... args：参数包的万能引用，保留参数的原始值类别（左值/右值）。

注意：这里的返回值是auto 是根据箭头后面的来推理出来的

- -> std::future<typename std::result_of<F(Args...)>::type>
- future用于异步获取进程的返回值
- result_of获取函数 F(Args...) 的返回值

```cpp
using return_type = typename std::result_of<F(Args...)>::type;
```

```cpp
auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
```
packaged_task包装，这个下面解释，先解释一下bind函数

bind函数，这里使用的很巧妙

这里可以看一下bind的用法，这个挺重要的 https://blog.csdn.net/qq_38410730/article/details/103637778

这里就介绍一下bind在这里的用法

对于一个函数

```cpp
void fun_1(int x,int y,int z) {
    std::cout << "print: x = " << x 
        << ", y = " << y 
        << ", z = " << z 
        << std::endl;
}
```
这里是有三个参数的，x y z ，我们通过 bind 来做处理

```cpp
//表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f2 的第一，二个参数指定
auto f1 = std::bind(fun_1, std::placeholders::_1, std::placeholders::_2, 3);
//调用
f1(1, 2);  //print: x=1,y=2,z=3 默认第三个参数为3

//这样的话：绑定函数 fun 的第一，二，三个参数值为： 1 2 3
auto f2 = std::bind(fun_1, 1, 2, 3); 	
//调用
f2();     //print: x=1,y=2,z=3 默认第三个参数为3
```
这里的用法是一样的：
```cpp
std::bind(std::forward<F>(f), std::forward<Args>(args)...)
```
实现了从有参到无参数的转化

继续:

```cpp
//从 packaged_task 中提取 future，用户可通过 res.get() 获取最终结果。
std::future<return_type> res = task->get_future();
```

这里解释一下上面说的 packaged_task 

std::packaged_task<return_type()> 的核心目的是将任意函数 ​​包装成异步任务​​，并自动关联一个 std::future 用于获取结果。

- std::packaged_task<return_type()> task(func); // 包装为 packaged_task
  - 此时，task 的签名是 return_type()，即调用 task() 会执行 func 并返回结果。
  - packaged_task 内部维护一个 ​​共享状态​​（shared state），当调用 task() 执行函数时：
    - 执行绑定的函数 func
    - 将函数返回值存入共享状态
    - 唤醒通过 future 等待该结果的线程
  
为什么上面的语句 未执行就能获取 future

std::future<return_type> res = task->get_future();
- future 是一个 ​​结果的占位符​​，它的有效性从 packaged_task 创建时就开始，与任务执行时机无关。调用 res.get() 时：
  - 若任务已执行：立即返回结果
  - 若任务未执行：阻塞等待结果

为什么要用make_shared管理

auto task = std::make_shared<std::packaged_task<return_type()>>(...);

- 确保 packaged_task 存活至任务被执行
- 不用手动释放(只能指针的妙用嘛)

继续：

```cpp
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    if(this->stop){
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    this->tasks.emplace([task]() { (*task)(); });  // 将任务包装为 void() 类型
}
```

前面不用多说，主要是最后一行

task经过之前的处理，已经相当于一个 函数指针 了，这里直接调用 (*task)() 他的返回值由 packaged_task 管理

我们整体来看：Lambda  表达式 [task]() { (*task)(); } 就是一个无参数，无返回值的函数 因此 可以正常的 放到 this->tasks里面

继续：

```cpp
    // 唤醒一个正在等待的 worker 线程来处理新任务。
    condition.notify_one();

    return res;
```
没啥好说的

我们最后再整体看一下代码：

```cpp
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{   
    // 获取 F 用 Args... 调用后的返回类型。
    using return_type = typename std::result_of<F(Args...)>::type;  //若 F 是 int foo(double), Args 是 double，则 return_type 是 int。
    // 将用户任务转换为 return_type() 签名（无参数，返回 return_type）
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );

    // 从 packaged_task 中提取 future，用户可通过 res.get() 获取最终结果。
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(this->stop){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        this->tasks.emplace([task]() { (*task)(); });  // 将任务包装为 void() 类型
    }
    // 唤醒一个正在等待的 worker 线程来处理新任务。
    condition.notify_one();

    return res;
}
```

整理流程：
- 用户调用 enqueue​
  - 提交函数 f 和参数 args，要求返回 future。
- 创建 packaged_task​
  - 将 f(args...) 绑定为 return_type() 类型的任务。
- 获取 future​
  - 立即通过 task->get_future() 获取结果占位符。
- ​​封装为 void() 任务​
  - 将任务包装为 [task](){ (*task)(); }，符合队列类型。
- ​​任务入队与通知​
  - 唤醒线程池中的工作线程。
- ​​任务执行​
  - 工作线程调用 (*task)()，执行用户函数并存储结果到共享状态。
- ​​用户获取结果​
  - 用户通过 future.get() 等待或获取结果。

## main测试
```cpp
#include <iostream>
#include "include/threadPool.h"
using namespace std;

int main(){
    // 测试1：基本任务执行和返回值
    {
        ThreadPool pool(2);
        auto future_add = pool.enqueue([](int a, int b) {
            return a + b;
        }, 2, 3);
        int result = future_add.get();
        std::cout << "Test 1: 2 + 3 = " << result << std::endl;  // 预期输出5
    }

    // 测试2：并行执行多个任务
    {
        const int num_tasks = 8;
        ThreadPool pool(4);
        std::vector<std::future<int>> futures;
        
        for (int i = 0; i < num_tasks; ++i) {
            futures.emplace_back(pool.enqueue([i] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟耗时操作
                return i * i;
            }));
        }

        std::cout << "Test 2: Results: ";
        for (auto& fut : futures) {
            std::cout << fut.get() << " ";  // 预期输出0 1 4 9 16 25 36 49（顺序可能不同）
        }
        std::cout << "\n";
    }

    // 测试3：异常传播
    {
        ThreadPool pool(2);
        auto future_exception = pool.enqueue([] {
            throw std::runtime_error("Intentional exception");
            return 0;
        });

        try {
            future_exception.get();
        } catch (const std::exception& e) {
            std::cout << "Test 3: Caught exception: " << e.what() << std::endl;
        }
    }

    // 测试4：线程池自动回收（通过作用域生命周期）
    {
        std::cout << "Test 4: Creating nested thread pool..." << std::endl;
        {
            ThreadPool inner_pool(2);
            inner_pool.enqueue([] {
                std::cout << "Task executed in inner pool" << std::endl;
            });
        }  // 此处inner_pool析构，自动等待任务完成
        std::cout << "Inner pool destroyed" << std::endl;
    }

    std::cout << "All tests completed successfully." << std::endl;
    return 0;
}
````

cd build

cmake .. 

make

./threadPool






