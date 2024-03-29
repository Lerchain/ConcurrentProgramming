﻿#include "pch.h"
#include <iostream>
#include <thread>
#include <stack>
#include <mutex>
#include <string>
#include <vector>
#include <stdio.h>
#include <exception>
#include <set>
using namespace std;

struct stack_empty:exception
{
	const char* wtf() throw()
	{
		return "empty stack";
	};
};


template<typename T>
class threadsafe_stack
{
private:
	stack<T> s;
	mutable mutex m;
public:
	threadsafe_stack<T>(threadsafe_stack<T>& os)
	{
		lock_guard<mutex> l(os.m);
		s = os;
	}
	threadsafe_stack<T>(stack<T>& os)
	{
		lock_guard<mutex> l(m);
		s = os;
	}
	threadsafe_stack<T>& operator=(const threadsafe_stack<T>&) = delete;
	void pop(T& res)
	{
		lock_guard<mutex> l(m);
		if (s.empty())
			throw stack_empty();
		res = s.top();
		s.pop();
	}
	shared_ptr<T> pop()
	{
		lock_guard<mutex> l(m);
		if (s.empty())
			throw stack_empty();
		shared_ptr<T> const res(make_shared<T>(s.top()));
		s.pop();
		return res;
	}
	void push(T val)
	{
		lock_guard<mutex> l(m);
		s.push(val);
	}
	bool empty()
	{
		lock_guard<mutex> l(m);
		if (s.empty())
			return true;
		return false;
	}
};
set<int> safev, unsafev;
mutex gm;
template<typename T>
void safework(threadsafe_stack<T>& safe)
{
	for (int i = 0; i < 5000; i++)
	{
		shared_ptr<T> sp = safe.pop();
		lock_guard<mutex> l(gm);
		safev.insert(*sp); //不上锁insert不是线程安全的，但上锁了push_back会导致数据顺序偶尔错乱。如果把锁移到循环开头则与单线程无异。总之就是不该用stl容器,threadsafe_stack没问题,换成set自动排序了也没问题。
	}
}
mutex gu;
template<typename T>
void unsafework(stack<T>& unsafe)
{
	for (int i = 0; i < 5000; i++)
	{
		T n = unsafe.top(); //异常走起
		unsafe.pop();
		lock_guard<mutex> l(gu);
		unsafev.insert(n);
	//	printf("%d ", n);
	}
}
void verify(set<int>& v)
{
	printf("V.size()=%d\n",v.size());
	vector<int> pos ;
	bool flag = 0;
	//for (auto it = v.begin(); it!=v.end(); it++)
	//{
	//	//printf("%d ", v[i]);
	//	if (v[i]+1 != v[i - 1])
	//	{
	//		flag = 1;
	//		pos.push_back(v[i]);
	//		pos.push_back(v[i - 1]);
	//		i--;
	//	}
	//}
	int val = -1;
	for (auto it = v.begin(); it != v.end(); it++)
	{
		if (*it != val + 1)
		{
			flag = 1;
			pos.push_back(val);
			pos.push_back(*it);
			it++;
		}
		val = *it;
	}
	if (flag)
	{
		cout << "Wrong\n";
		for (auto x : pos)
			cout << x << " ";
	}
	else
	printf("true\n");
}
int main()
{
	stack<int> unsafe;
	for (int i = 0; i < 10000; i++)
	{
		unsafe.push(i);
	}
	threadsafe_stack<int> safe(unsafe);
//	vector<thread> threads;
/**								//错误测试方法
	for (int i = 0; i < 500; i++)
	{
	//	threads.push_back(thread([&safe, &unsafe, i] { unsafe.pop(); unsafe.pop();  }));
	//	threads.push_back(thread([&safe, &unsafe, i] { safe.pop(); safe.pop();}));
		threads.push_back(thread([&safe, &unsafe, i] { unsafework(unsafe); unsafework(unsafe); }));
	}

	for (int i = 0; i < 500; i++)
	{
		threads[i].detach();
	}
**/
	//thread t(unsafework<int>, ref(unsafe));
	//thread t1(unsafework<int>, ref(unsafe));    
	thread ts(safework<int>, ref(safe)); 
	thread ts1(safework<int>, ref(safe));

	//t1.join();
	//t.join();
	ts.join();
	ts1.join();
	//verify(unsafev);
	verify(safev);
}

