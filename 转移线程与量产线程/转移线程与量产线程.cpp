#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <stdio.h>
using namespace std;

class ThreadGuard
{
	thread t;
public:
	explicit ThreadGuard(thread _t) :t(move(_t))
	{
		if (!t.joinable())
		{
			throw "This thread is NULL";
		}
	}
	~ThreadGuard()
	{
		t.join();
	}
	ThreadGuard(ThreadGuard&) = delete;
	ThreadGuard& operator=(ThreadGuard&) = delete;
};
void Origin(const string& s)
{
	cout << s <<" Original thread" << endl;
}
void ThreadGuardMoveText(const string& s)
{
	cout << s << "Test" << endl;
}
void doSomething(int i)
{
	printf("%d ", i);
	printf("do something");
}
int main()
{
	string s = "Hello";
	thread t1(Origin, s);
	thread t2 = move(t1);
	t2.join();
	cout << "This is main thread" << endl;
	string s2 = "This is ThreadGuard move";
	ThreadGuard g(thread(ThreadGuardMoveText,s2));
	vector<thread> threads;
	for (int i = 0; i < 20; i++)
	{
		threads.push_back(thread(doSomething, i));
	}
	for (int i = 0; i < 20; i++)
	{
		threads[i].join();
	}
}


