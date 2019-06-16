#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <list>
#include <stdio.h>
using namespace std;

//一定要保证要保护的数据仅在互斥锁作用域内，不能将其指针或引用暴露出去。
void Maliciousfunc(list<int>& data);
class Badclass
{
	mutex m;
	list<int> data;
public:
	template<typename func>
	void Foolishfunc(func _func)
	{
		lock_guard<mutex> l(m); //看似锁住了
		_func(data); //func函数绕过锁修改data
	}
	void add_data(int val)
	{
		lock_guard<mutex> l(m);
		data.push_back(val);
	}
	void show_data()
	{
		lock_guard<mutex> l(m);
	//	printf_s("%d\n", data.size());
		for (auto x : data)
			printf_s("%d ",x);
		printf_s("\n");
	}
};
list<int> * unprotectedData;
void Maliciousfunc(list<int>& data)
{
	unprotectedData = &data; //外部指针指向data
}
void foo(Badclass& x,int i)
{
	x.Foolishfunc(Maliciousfunc); 
	unprotectedData->push_back(i);//修改
}
int main()
{
	list<int> data(10);
	int i = 0;
	for (auto it = data.begin(); it!=data.end(); it++)
	{
		*it = i++;
	}
	vector<thread> threads;
	Badclass b;
	for (int i = 0; i < 10; i++)
	{
		threads.push_back(thread([&b, i] {b.add_data(i); b.show_data(); }));
//		threads.push_back(thread([&b, i] {b.add_data(i);}));
		foo(b,-i-1);
//		printf_s("\n");
	}
	for (int  i = 0; i < 10; i++)
	{
		threads[i].join();
	}
	printf_s("\n");
    b.show_data();
}

