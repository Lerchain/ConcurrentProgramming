#include "pch.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <windows.h>
#include <mutex>
using namespace std;

class spinlock_mutex
{
	atomic_flag flag = ATOMIC_FLAG_INIT;
public:
	spinlock_mutex(){}
	void lock()
	{
		while (flag.test_and_set(memory_order_acquire));
	}
	void unlock()
	{
		flag.clear(memory_order_release);
	}
};

spinlock_mutex m;
void Test()
{
	lock_guard<spinlock_mutex> lk(m);
	for (int i = 0; i < 10; i++)
	{
		cout << i << endl;
		Sleep(100);
	}
}
int main()
{
	thread t1(Test);
	thread t2(Test);
	t1.join();
	t2.join();
}

