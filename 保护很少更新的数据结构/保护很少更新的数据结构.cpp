

#include "pch.h"
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <map>
#include <string>
#include <time.h>

using namespace std;

template<typename T,typename U>
class UsuallyReadObject
{
	map<T,U> m;
	shared_mutex sm;
public:
	UsuallyReadObject(map<T,U> _m):m(_m)
	{}
	U getData(T x)
	{
		shared_lock<shared_mutex> lk(sm);
		return m.at(x);
	}
	void modifyData(T first, U second)
	{
		lock_guard<shared_mutex> lk(sm);
		time_t initime = time(0);
		while (time(0) - initime < 5)   //这是最佳测试办法，每次更改都得等3S，在等的这段时间，不会读。证明确实锁住了。
		m[first] = second;
	}
};
void Read(UsuallyReadObject<int,string>& u)
{
	time_t initime = time(0);
	while (time(0) - initime< 1)
	{
		for (int i = 0; i < 10; i++)
		{
			cout << u.getData(i) + " ";
		}
	}
}
void Write(UsuallyReadObject<int, string>& u)
{
	time_t initime = time(0);
	int j = 1;
	//while (time(0) - initime < 1)
	//{
		for (int i = 0; i < 10; i++)
		{
			u.modifyData(i, to_string(i+j*10));
		}
		//j++;
	//}
}
int main()
{
	map<int, string> m;
	for (int i = 0; i < 10; i++)
	{
		m[i] = to_string(i);
	}
	UsuallyReadObject<int,string> u(m);
	thread t1(Read, ref(u));
	thread t3(Read, ref(u));	//前两个读共享，不阻塞
	thread t2(Write, ref(u));   //写阻塞，全写完后两个继续读。
	t1.join();
	t2.join();
	t3.join();
}