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