#include "pch.h"
#include <iostream>           
#include <thread>             
#include <mutex>              
#include <condition_variable> 
#include <queue>
#include <windows.h>

using std::cout;
using std::mutex;
using std::thread;
using std::condition_variable;
using std::unique_lock;
using std::queue;
using std::lock_guard;
using std::shared_ptr;
template<typename T>
class thread_safeQueue
{
	queue<T> q;
//	mutable mutex m;
	mutex m;
	condition_variable cv;
public:
	thread_safeQueue(){}
	thread_safeQueue(queue<T>& _q):q(_q){}
	thread_safeQueue(thread_safeQueue const & tq)
	{
		lock_guard<mutex> lk(tq.m);
		q = tq;
	}
	thread_safeQueue& operator= (thread_safeQueue const &) = delete;
	bool empty() const
	{
		lock_guard<mutex> lk(m);
		if (q.empty())
			return true;
		return false;
	}
	void push(T some_data)
	{
		lock_guard<mutex> lk(m);
		q.push(some_data);
		cv.notify_one();
	}
	void waitPop(T& front_data)
	{
		unique_lock<mutex> lk(m);
		cv.wait(lk, [this] {return !q.empty(); });
		front_data = q.front();
		q.pop();
	}
	shared_ptr<T>& waitPop()
	{
		unique_lock<mutex> lk(m);
		cv.wait(lk, [this] {return !q.empty(); });
		shared_ptr<T> p(std::make_shared<T>(q.front()));
		q.pop();
		return p;
	}
	bool tryPop(T& front_data)
	{
		lock_guard<mutex> lk(m);
		if (q.empty())
			return false;
		front_data = q.front();
		q.pop();
	}
	shared_ptr<T>& tryPop()
	{
		lock_guard<mutex> m;
		if (q.empty())
			return nullptr;
		shared_ptr<T> p(std::make_shared<T>(q.front));
		q.pop();
		return p;
	}
};

void dataProcess(thread_safeQueue<int>& sq)
{
	while (1)
	{
		int data;
		sq.waitPop(data);
		printf("%d ", data);
	}
}

void dataPrepair(thread_safeQueue<int>& sq)
{
	int i = 1;
	while (1)
	{
		Sleep(1000);
		sq.push(i++);
	}
}

void mutableTest(thread_safeQueue<int>& sq)
{
	while (1)
	{
		sq.empty();
	}
}
int main()
{
	thread_safeQueue<int> sq;
	thread t1(dataProcess, std::ref(sq));
	thread t2(dataPrepair, std::ref(sq));
//	thread mt(mutableTest, std::ref(sq)); 
//事实上想测试mutable用mutableTest就够了不用为此在创建个thread，如果只是单纯的定义了empty（任何函数中都没调用它），编译器将不会报错。
//反之，编译器会在编译时直接报错。
	t1.join();
	t2.join();
//	mt.join();
	
}
