
#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
using namespace std;

class ThreadGuard
{
	thread& t;
public:
	explicit ThreadGuard(thread& _t):t(_t)
	{
	}
	~ThreadGuard()
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	ThreadGuard(ThreadGuard&) = delete;
	ThreadGuard& operator=(ThreadGuard&) = delete;
};
void func(const string& Aword)
{
	cout << Aword << endl;
}
class chuanzhizhen {
public:
	void eg(const string& s)
	{
		cout << s << endl;
	}
};
int main()
{
	string s = "This is a word";
	thread t(func, ref(s)); //ref 保证是真的传地址。
	ThreadGuard g(t);
	cout << "This is Main thread" << endl;
	chuanzhizhen hehe;
	string s1 = "This is eg\n";
	thread t2(&chuanzhizhen::eg, &hehe, s1);
	ThreadGuard k(t2);
}

