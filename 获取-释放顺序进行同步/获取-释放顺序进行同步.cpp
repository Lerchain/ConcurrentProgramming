#include "pch.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <Windows.h>
#include <mutex>
#include <chrono>
#include <assert.h>
using namespace std;

atomic<bool> x=false, y = false, start = false;
atomic<int> X1;
int X,Y;

void writeX()
{
	while (!start.load(memory_order_relaxed));
	X++;
	X1.store(X1 + 1, memory_order_relaxed);
	x.store(true, memory_order_release);
}

void writeY()
{
	while (!start.load(memory_order_relaxed));
	Y++;
	y.store(true, memory_order_release);
}

void readYthenX()
{
	while (!start.load(memory_order_relaxed));
	while (!y.load(memory_order_acquire));
	if (x.load(memory_order_acquire))
		cout << "X has synchronized";
	else
		cout << "X does not have synchronzed";
	cout << X;
	cout << X1.load(memory_order_relaxed);
}

atomic<bool> sync1 = false, sync2 = false, go = false;
atomic<int> data1, data2, data3, data4, data5;

void thread1()
{
	while (!go.load());
	data1.store(1,memory_order_relaxed);
	data2.store(2, memory_order_relaxed);
	data3.store(3, memory_order_relaxed);
	data4.store(4, memory_order_relaxed);
	data5.store(5, memory_order_relaxed);
//	sync1.store(true, memory_order_release);	
	sync1.store(true, memory_order_relaxed);//我发现用relaxed存，也不会乱序。
//查了下，原来X86 是个强类型保证的CPU。意思是它能保证代码的顺序执行，也就是不会乱序。
}

void thread2()
{
	while (!go.load());
//	while (!sync1.load(memory_order_acquire));
	while(!sync1.load(memory_order_relaxed));
//	sync2.store(memory_order_release);			//我去了，我这里失误没有存true,为什么还会行。
	sync2.store(true,memory_order_relaxed);

}

void thread3()
{
	while (!go.load());
	while (!sync2.load(memory_order_relaxed));
	assert(data1.load(memory_order_relaxed) == 1);
	assert(data2.load(memory_order_relaxed) == 2);
	assert(data3.load(memory_order_relaxed) == 3);
	assert(data4.load(memory_order_relaxed) == 4);
	assert(data5.load(memory_order_relaxed) == 5);
	cout << "All right" << endl;
}

int main()
{
	thread t3(readYthenX); // 这个线程顺序稍显重要，毕竟程序简单，初始化线程不是同时的。
	thread t1(writeX);
	thread t2(writeY);
	Sleep(1000);
	start = true; //所以让它们同时启动。
	t1.join();
	t2.join();
	t3.join();
	for (int i = 0; i < 100; i++)
	{
		thread T1(thread1);
		thread T2(thread2);
		thread T3(thread3);
		go = true;
		T1.join();
		T2.join();
		T3.join();
		go = false;
		sync1 = false;
		sync2 = false;
		data1 = 0;
		data2 = 0;
		data3 = 0;
		data4 = 0;
		data5 = 0;
	}


}