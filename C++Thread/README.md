# C++线程

## 基本介绍：

1.多线程的含义

多线程（multithreading），是指在软件或者硬件上实现多个线程并发执行的技术。具有多核CPU的支持的计算机能够真正在同一时间执行多个程序片段，进而提升程序的处理性能。在一个程序中，这些独立运行的程序片段被称为“线程”（Thread），利用其编程的概念就叫作“多线程处理”。

2.进程与线程的区别

进程是指一个程序的运行实例，而线程是指进程中独立的执行流程。一个进程可以有多个线程，多个线程之间可以并发执行。

- 一个程序有且只有一个进程，但可以拥有至少一个的线程。

- 不同进程拥有不同的地址空间，互不相关，而不同线程共同拥有相同进程的地址空间。

## 线程创建

thread

```cpp
#include<iostream>
#include<thread>
using namespace std;

void ThreadFunctionA(int A){
    for(int i=0;i<A;i++){
        cout<<"线程 A："<<i<<endl;
    }
}

void ThreadFunctionB(int B){
    for(int i=0;i<B;i++){
        cout<<"线程 B："<<i<<endl;
    }
}

int main(){

    thread ThreadA(ThreadFunctionA,10);
    thread ThreadB(ThreadFunctionB,5);

    ThreadA.join();ThreadB.join();
    
    return 0;
}
```


## join() 和 detach()

在C++中，创建了一个线程时，它通常被称为一个可联接(joinable)的线程，可以通过调用join()函数或detach()函数来管理线程的执行。

| 方法       | 说明                                                                 |
|------------|----------------------------------------------------------------------|
| join()     | 等待一个线程完成，如果该线程还未执行完毕，则当前线程（一般是主线程）将被阻塞，直到该线程执行完成，主线程才会继续执行。 |
| detach()   | 将当前线程与创建的线程分离，使它们分别运行，当分离的线程执行完毕后，系统会自动回收其资源。如果一个线程被分离了，就不能再使用 join() 函数了，因为线程已经无法被联接了。 |
| joinable() | 判断线程是否可以执行 join() 函数，返回 true/false                     |

注意：

- 线程是在thread对象被定义的时候开始执行的，而不是在调用join()函数时才执行的，调用join()函数只是阻塞等待线程结束并回收资源。

- 分离的线程（执行过detach()的线程）会在调用它的线程结束或自己结束时自动释放资源。

- 线程会在函数运行完毕后自动释放，不推荐利用其他方法强制结束线程，可能会因资源未释放而导致内存泄漏。

- 若没有执行join()或detach()的线程在程序结束时会引发异常。

## 当前线程的功能函数 this_thread

| 使用                                      | 说明                                     |
|-------------------------------------------|------------------------------------------|
| `std::this_thread::sleep_for()`            | 当前线程休眠指定的时间                   |
| `std::this_thread::sleep_until()`          | 当前线程休眠直到指定时间点               |
| `std::this_thread::yield()`                | 当前线程让出CPU，允许其他线程运行         |
| `std::this_thread::get_id()`               | 获取当前线程的ID                         |


```cpp

#include<iostream>
#include<thread>
#include<chrono>
using namespace std;

void my_thread(){
    cout<<"Thread ID"<<this_thread::get_id()<<" start! "<<endl;
    for (int i = 0; i < 5; i++){
        cout << "Thread " << this_thread::get_id() << " running: " << i << endl;
        this_thread::yield();	// 让出当前线程的时间片
        this_thread::sleep_for(chrono::milliseconds(200));  // 线程休眠200毫秒
    }
    cout << "Thread " << this_thread::get_id() << " end！" << endl;
}

int main(){

    cout << "Main thread id: " << this_thread::get_id() << endl;
	
    thread t1(my_thread);
	thread t2(my_thread);
	
	t1.join();
	t2.join();

    return 0;
}
```


## std::mutex

在多线程编程中，需要注意以下问题：

线程之间的共享数据访问需要进行同步，以防止数据竞争和其他问题。可以使用互斥量、条件变量等机制进行同步。
可能会发生死锁问题，即多个线程互相等待对方释放锁，导致程序无法继续执行。
可能会发生竞态条件问题，即多个线程执行的顺序导致结果的不确定性。


std::mutex是 C++11 中最基本的互斥量，一个线程将mutex锁住时，其它的线程就不能操作mutex，直到这个线程将mutex解锁。

**1.lock() 与 unlock()**

| 方法      | 说明                                                                 |
|-----------|----------------------------------------------------------------------|
| lock()    | 将mutex上锁。如果mutex已经被其它线程上锁，那么会阻塞，直到解锁；如果mutex已经被同一个线程锁住，那么会产生死锁。 |
| unlock()  | 将mutex解锁，释放其所有权。如果有线程因为调用 lock() 不能上锁而被阻塞，则调用此函数会将mutex的主动权随机交给其中一个线程；如果mutex不是被此线程上锁，那么会引发未定义的异常。 |
| try_lock()| 尝试将mutex上锁。如果mutex未被上锁，则将其上锁并返回true；如果mutex已被锁则返回false。 |


```cpp
//这样其实还不明显，比如写1+到10000,分10个线程运行，加不加锁就非常明显了
#include<iostream>
#include<thread>
#include<mutex>
using namespace std;

mutex mtx;
int num = 0;


void thread_function(int &n){
    for(int i=0;i<100;i++){
        mtx.lock();
        n++;
        mtx.unlock();
    }
}

int main(){

    thread myThread[500];

    for(thread &a:myThread){
        a = thread(thread_function,ref(num));
        a.join();
    }
    cout << "num = " << num << endl;
	cout << "Main thread exits!" << endl;

    return 0;
}
```


**2.lock_guard**

std::lock_guard是C++标准库中的一个模板类，用于实现资源的自动加锁和解锁。它是基于RAII（资源获取即初始化）的设计理念，能够确保在作用域结束时自动释放锁资源，避免了手动管理锁的复杂性和可能出现的错误。

std::lock_guard的主要特点如下：

- 自动加锁： 在创建std::lock_guard对象时，会立即对指定的互斥量进行加锁操作。这样可以确保在进入作用域后，互斥量已经被锁定，避免了并发访问资源的竞争条件。

- 自动解锁：std::lock_guard对象在作用域结束时，会自动释放互斥量。无论作用域是通过正常的流程结束、异常抛出还是使用return语句提前返回，std::lock_guard都能保证互斥量被正确解锁，避免了资源泄漏和死锁的风险。
- 适用于局部锁定： 由于std::lock_guard是通过栈上的对象实现的，因此适用于在局部范围内锁定互斥量。当超出std::lock_guard对象的作用域时，互斥量会自动解锁，释放控制权。

使用std::lock_guard的一般步骤如下：

- 创建一个std::lock_guard对象，传入要加锁的互斥量作为参数。
执行需要加锁保护的代码块。

- std::lock_guard对象的作用域结束时，自动调用析构函数解锁互斥量。

```cpp
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex mtx;  // 互斥量

void thread_function()
{
    lock_guard<mutex> lock(mtx);  // 加锁互斥量
    cout << "Thread running" << endl;
    // 执行需要加锁保护的代码
}  // lock_guard对象的析构函数自动解锁互斥量

int main()
{
    thread t1(thread_function);
    t1.join();
    cout << "Main thread exits!" << endl;
    return 0;
}


```

在上述示例中，std::lock_guard对象lock会在thread_function中加锁互斥量，保护了输出语句的执行。当thread_function结束时，lock_guard对象的析构函数会自动解锁互斥量。这样可以确保互斥量在合适的时候被锁定和解锁，避免了多线程间的竞争问题。

总而言之，std::lock_guard提供了一种简单而安全的方式来管理互斥量的锁定和解锁，使多线程编程更加方便和可靠。


**3.unique_lock**

std::unique_lock是C++标准库中的一个模板类，用于实现更加灵活的互斥量的加锁和解锁操作。它提供了比std::lock_guard更多的功能和灵活性。

std::unique_lock的主要特点如下：

- 自动加锁和解锁： 与std::lock_guard类似，std::unique_lock在创建对象时立即对指定的互斥量进行加锁操作，确保互斥量被锁定。在对象的生命周期结束时，会自动解锁互斥量。这种自动加锁和解锁的机制避免了手动管理锁的复杂性和可能出现的错误。

- 支持灵活的加锁和解锁： 相对于std::lock_guard的自动加锁和解锁，std::unique_lock提供了更灵活的方式。它可以在需要的时候手动加锁和解锁互斥量，允许在不同的代码块中对互斥量进行多次加锁和解锁操作。

- 支持延迟加锁和条件变量：std::unique_lock还支持延迟加锁的功能，可以在不立即加锁的情况下创建对象，稍后根据需要进行加锁操作。此外，它还可以与条件变量（std::condition_variable）一起使用，实现更复杂的线程同步和等待机制。

使用std::unique_lock的一般步骤如下：

- 创建一个std::unique_lock对象，传入要加锁的互斥量作为参数。

- 执行需要加锁保护的代码块。

- 可选地手动调用lock函数对互斥量进行加锁，或者在需要时调用unlock函数手动解锁互斥量。

```cpp
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;
mutex mtx;  // 互斥量

void thread_function()
{
    unique_lock<mutex> lock(mtx);  // 加锁互斥量
    cout << "Thread running" << endl;
    // 执行需要加锁保护的代码
    lock.unlock();  // 手动解锁互斥量
    // 执行不需要加锁保护的代码
    lock.lock();  // 再次加锁互斥量
    // 执行需要加锁保护的代码
}  
// unique_lock对象的析构函数自动解锁互斥量

int main()
{
    thread t1(thread_function);
    t1.join();
    cout << "Main thread exits!" << endl;
    return 0;
}

```


## condition_variable

std::condition_variable是C++标准库中的一个类，用于在多线程编程中实现线程间的条件变量和线程同步。它提供了等待和通知的机制，使得线程可以等待某个条件成立时被唤醒，或者在满足某个条件时通知其他等待的线程。其提供了以下几个函数用于等待和通知线程：

| 方法         | 说明                                                                 |
|--------------|----------------------------------------------------------------------|
| wait()       | 使当前线程进入等待状态，直到被其他线程通过 notify_one() 或 notify_all() 函数唤醒。该函数需要一个互斥锁作为参数，调用时会自动释放互斥锁，并在被唤醒后重新获取互斥锁。 |
| wait_for()   | 使当前线程进入等待状态，最多等待一定的时间，直到被其他线程通过 notify_one() 或 notify_all() 函数唤醒，或者等待超时。该函数需要一个互斥锁和一个时间段作为参数，返回时有两种情况：等待超时返回 `std::cv_status::timeout`，被唤醒返回 `std::cv_status::no_timeout`。 |
| wait_until() | 使当前线程进入等待状态，直到被其他线程通过 notify_one() 或 notify_all() 函数唤醒，或者等待时间达到指定的绝对时间点。该函数需要一个互斥锁和一个绝对时间点作为参数，返回时有两种情况：时间到达返回 `std::cv_status::timeout`，被唤醒返回 `std::cv_status::no_timeout`。 |
| notify_one() | 唤醒一个等待中的线程，如果有多个线程在等待，则选择其中一个线程唤醒。                     |
| notify_all()  | 唤醒所有等待中的线程，使它们从等待状态返回。                                       |


std::condition_variable的主要特点如下：

- 等待和通知机制：std::condition_variable允许线程进入等待状态，直到某个条件满足时才被唤醒。线程可以调用wait函数进入等待状态，并指定一个互斥量作为参数，以确保线程在等待期间互斥量被锁定。当其他线程满足条件并调用notify_one或notify_all函数时，等待的线程将被唤醒并继续执行。

- 与互斥量配合使用：std::condition_variable需要与互斥量（std::mutex或std::unique_lock\<std::mutex\>）配合使用，以确保线程之间的互斥性。在等待之前，线程必须先锁定互斥量，以避免竞争条件。当条件满足时，通知其他等待的线程之前，必须再次锁定互斥量。

- 支持超时等待：std::condition_variable提供了带有超时参数的等待函数wait_for和wait_until，允许线程在等待一段时间后自动被唤醒。这对于处理超时情况或限时等待非常有用

使用std::condition_variable的一般步骤如下：

- 创建一个std::condition_variable对象。

- 创建一个互斥量对象（std::mutex或std::unique_lock<std::mutex>）。

- 在等待线程中，使用std::unique_lock锁定互斥量，并调用wait函数进入等待状态。

- 在唤醒线程中，使用std::unique_lock锁定互斥量，并调用notify_one或notify_all函数通知等待的线程。

- 等待线程被唤醒后，继续执行相应的操作。

```cpp

#include <iostream>
#include <thread>
#include <condition_variable>
using namespace std;

mutex mtx;  // 互斥量
condition_variable cv;  // 条件变量
bool isReady = false;  // 条件

void thread_function()
{
    unique_lock<mutex> lock(mtx);
    while (!isReady) 
    {
        cv.wait(lock);  // 等待条件满足
    }
    cout << "Thread is notified" << endl;
}

int main()
{
    thread t(thread_function);

    // 模拟一段耗时操作
    this_thread::sleep_for(chrono::seconds(2));

    {
        lock_guard<mutex> lock(mtx);
        isReady = true;  // 设置条件为true
    }
    cv.notify_one();  // 通知等待的线程

    t.join();

    return 0;
}

```

## atomic

std::mutex可以很好地解决多线程资源争抢的问题，但它每次循环都要加锁、解锁，这样固然会浪费很多的时间。

在 C++ 中，std::atomic 是用来提供原子操作的类，atomic，本意为原子，原子操作是最小的且不可并行化的操作。这就意味着即使是多线程，也要像同步进行一样同步操作原子对象，从而省去了互斥量上锁、解锁的时间消耗。

使用 std::atomic 可以保证数据在操作期间不被其他线程修改，这样就避免了数据竞争，使得程序在多线程并发访问时仍然能够正确执行。


```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>   //必须包含
using namespace std;

atomic_int num = 0;

void thread_function(atomic_int &n)  //修改类型
{
	for (int i = 0; i < 100; ++i)
	{
		n++;
	}
}

int main()
{
	thread myThread[500];
	for (thread &a : myThread)
	{
		a = thread(thread_function, ref(num));
		a.join();
	}

	cout << "num = " << num << endl;
	cout << "Main thread exits!" << endl;
    
	return 0;
}


```
