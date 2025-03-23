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
