#include "pch.h"
#include <iostream>
#include <thread>
using namespace std;
class ThreadGuard
{
	thread& t;
public:
	explicit ThreadGuard(thread& t_):
		t(t_)
	{}
	~ThreadGuard()
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	ThreadGuard(ThreadGuard& const) = delete; //不允许拷贝构造函数
	ThreadGuard& operator=(ThreadGuard const&) = delete;//不允许拷贝赋值操作
};
void helloworld()
{
	cout << "This is Hello World"<<endl;
}
void doSomething()
{
	for (int i = 0; i < 5; i++)
	{
		cout << i << endl;
	}
	cout << "This is \"do something\""<<endl;
}
int main()
{
	thread t([] {helloworld(); doSomething(); });
	ThreadGuard k(t);
	cout << "Main have an exception" << endl;//出现了异常也能保证线程执行完，因为会先析构ThreadGuard类
	throw("Oops");
}