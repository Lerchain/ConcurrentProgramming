#include "pch.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
using namespace std;

class Data
{
private:
	int id;
public:
	Data(int _id):id(_id){}
	void show()
	{
		cout << to_string(id) + " ";
	}
};
class DataProcess
{
	mutex m;
	condition_variable cv;
	queue<Data> q;
//	DataProcess(){}
public:
	void prepairData(Data someData)
	{
		lock_guard<mutex> lk(m);
		q.push(someData);
		cv.notify_one();
	}
	void processData()
	{
		unique_lock<mutex> lk(m);
		cv.wait(lk, [&] {return !q.empty(); });
		Data t = q.front();
		q.pop();
		lk.unlock();
		t.show();
	}
	/*static DataProcess& getInstance()
	{
		static DataProcess p;
		return p;
	}
	DataProcess(DataProcess& D):m(move(D.m)),
	{

	}*/
};

/*	这个例子中，我创建了一个处理数据过程的类，多线程并发条件下，必须保证向处理队列中压入数据的过程、 处理数据的过程都是不可并发的，需要上锁保护，
	将要处理的数据放入处理队列中好说，只需要正常上锁就行了，但处理数据的方法该如何上锁呢，如果也只是单纯的上锁，多线程并发条件下，你锁住了这个互斥量，
	而处理队列里没有东西，你就得一直等待，而互斥量已经被锁住了，其他线程也没法往处理队列中加东西，这就死锁了。所以最好的办法就是每当往处理队列中加一个
	东西，通过条件变量通知一下，所有正在用此条件变量等待的线程都会check一下是否满足了条件，满足就会占用锁了，开始接下来的处理，不满足就不占用锁，继续
	等待下一次通知变量的传达。
*/
void processData(DataProcess& D)
{
	int i = 100;
	while (i--)
	{
		D.processData();
	}
}
void pushData(DataProcess& D)
{
	for (int i = 0; i < 100; i++)
	{
		D.prepairData(i);
	}
}
class ThreadGuard
{
	thread& t;
public:
	ThreadGuard(thread& _t):t(_t){}
	ThreadGuard(ThreadGuard&) = delete;
	ThreadGuard& operator=(ThreadGuard&) = delete;
	~ThreadGuard()
	{
		if (t.joinable())
			t.join();
	}
};

int main()
{
//	DataProcess D = DataProcess::getInstance();  最大的问题是这个类好像不能用Singlton,mutex不能move。
	DataProcess D;
	thread t1(processData, ref(D));
	thread t2(pushData, ref(D));
	ThreadGuard T1(t1);
	ThreadGuard T2(t2);
}
