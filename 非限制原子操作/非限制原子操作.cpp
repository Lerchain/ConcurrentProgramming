#include "pch.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
using namespace std;

atomic<int> x = 0, y = 0, z = 0;
atomic<bool> start = false;

struct Value
{
	int xval, yval, zval;
};

Value values[5][10];

void atomicPlusOne(atomic<int>* a, Value v[])
{
	while (!start.load())
		this_thread::yield();
	for (int i = 0; i < 10; i++)
	{
		v[i].xval = x.load(memory_order_relaxed);
		v[i].yval = y.load(memory_order_relaxed);
		v[i].zval = z.load(memory_order_relaxed);
		a->store(i + 1, memory_order_relaxed);
		this_thread::yield();
	}
}

void atomicRead(Value v[])
{
	while (!start.load())
		this_thread::yield();
	for (int i = 0; i < 10; i++)
	{
		v[i].xval = x.load(memory_order_relaxed);
		v[i].yval = y.load(memory_order_relaxed);
		v[i].zval = z.load(memory_order_relaxed);
	}
}

void print(Value v[])
{
	for (size_t i = 0; i < 10; i++)
	{
		cout << "(" << v[i].xval << ", " << v[i].yval << ", " << v[i].zval << ")" << "  ";
	}
}

int main()
{
	thread t1(atomicPlusOne, &x, values[0]);
	thread t2(atomicPlusOne, &y, values[1]);
	thread t3(atomicPlusOne, &z, values[2]);
	thread t4(atomicRead, values[3]);
	thread t5(atomicRead, values[4]);
	start.store(true);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	for (int i = 0; i < 5; i++)
	{
		print(values[i]);
		cout << endl;
	}
}