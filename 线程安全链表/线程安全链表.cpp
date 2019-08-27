#include "pch.h"
#include "ThreadSafeList.hpp"
#include <thread>
#include <atomic>
using namespace std;
using std::atomic_bool;
atomic_bool start = false;
void func(int val)
{
	printf("%d", val);
}
void fEach(ThreadSafeList<int>& l)
{
	while (!start.load());
	l.forEach(func);
}
int main()
{
	ThreadSafeList<int> l;
	thread t([&]
	{
		while (!start.load());
		for (int i = 0; i < 10; i++)
		{
			l.pushFront(i);
		}
	});
	start = true;
	t.join();
	//thread t2(fEach,ref(l));
	thread t2(
	[&l]
	{
		l.forEach(
		[&](int val)
		{
			printf("%d ", val);
		}
		);
	}
	);
	t2.join();
	cout << endl;
	thread t3(
	[&l]
	{
		shared_ptr<int>	p = l.findFisrstIf(
		[&](int val)
		{
			if (val == 6)
				return true;
			else
				return false;
		}
		);
		if (p)
			cout << *p;
	}
	);
	t3.join();
	thread t4(
		[&l]
		{
		l.removeif(
			[&](int val) {
			if (val == 4)
				return true;
			else
				return false;
		});
		fEach(l);
	});
	cout << endl;
	t4.join();
}
