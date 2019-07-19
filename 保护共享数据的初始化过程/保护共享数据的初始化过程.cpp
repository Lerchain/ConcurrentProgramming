#include "pch.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>

using namespace std;

int cnt;
class Data
{
	string name[5]{};
	int id[5]{};
	mutex m;
public:
	Data()
	{
		for (int i = 0; i < 5; i++)
		{
			name[i] = to_string(i+1);
			id[i] = i+1;
 		}
	}
	void setData(string name[],int id[])
	{
		for (int i = 0; i < 5; i++)
		{
			this->name[i] = name[i];
			this->id[i] = id[i];
		}
	}
	void showData()
	{
		for (int i = 0; i < 5; i++)
		{
			printf("Id%d\n", id[i]);
		}
	}
	void verify()
	{
		for (int i = 0; i < 5; i++)
		{
			if (id[i] != i + 1)
				printf("False");
		}
		lock_guard<mutex> l(m);
		++cnt;
	}
};


class SomeObject
{
	Data* someDetails;
	mutex m;
	once_flag o;
public:
	void badlyShowSomeDetail()
	{
		if (!someDetails)
		{
			unique_lock<mutex> lk(m);
			if (!someDetails)
			{
				someDetails = new Data;
			}
		}
		someDetails->verify();
//	someDetails->showData();
	}
	void initSomeDetails()
	{
		someDetails = new Data;
	}
	void betterShowSomeDetail()
	{
		call_once(o,&SomeObject::initSomeDetails,this);
		//someDetails->showData();
		someDetails->verify();
	}
	void badlySetSomeDetails()
	{
		string name[5]{ "oops1","oops2","oops3","oops4","oops5" };
		int id[5]{ 5,4,3,2,1 };
		if (!someDetails)
		{
			unique_lock<mutex> lk(m);
			if (!someDetails)
			{
				someDetails = new Data;
			}
		}
		someDetails->setData(name, id);
	}
	void betterSetSomeDetails()
	{
		string name[5]{ "oops1","oops2","oops3","oops4","oops5" };
		int id[5]{ 5,4,3,2,1 };
		call_once(o, &SomeObject::initSomeDetails, this);
		someDetails->setData(name, id);
	}

};

//写的又有点懵了，解释下我写的两个class与我想测试表达的意思。
//首先，并发的两个线程，想要延迟初始化，初始化后就隐式同步了，也就是说只有初始化的过程是需要互斥量保护的。
//那我该如何测试呢，首先要明确的就是双重检查锁模式是可以保证延迟初始化与线程安全性的，并且避免了线程的序列化。
//但之所以不好是因为它暗含一个险恶的数据竞争。锁只锁住了初始化的过程，第一次检查指针是否为空是没有锁的，而这时如果另一个线程刚好在给指针赋予指向的对象，
//但还没赋完，判断语句会认为指针不是空的(确实不是空的，但也没有指向正确的对象)，然后指针再showData或者做些事就GG了。
//这么解释后，测试就明朗了，我只需要创建两个线程，创建多个someObject类，每个线程里把多个类中的badlyShowSomeDetails执行一遍，总会有一个类出现上面的数据竞争。
//Let's do it.

void Test(vector<SomeObject>& t)
{
	int n = t.size();
	for (int i = 0; i < n; i++)
	{
		t[i].badlyShowSomeDetail();
	}
}
void Test2(vector<SomeObject>& T)
{
	int n = T.size();
	for (int i = 0; i < n; i++)
	{
		T[i].betterShowSomeDetail();
	}
}
int main()
{
	vector<SomeObject> t(10000);
	thread t1(Test, ref(t));
	thread t2(Test, ref(t));
	thread t3(Test, ref(t));
	thread t4(Test, ref(t));
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	cout << cnt<<endl;
	//哈哈哈哈成功咯，我最开始是正常show，发现show不出什么，因为多线程并发，id顺序错乱是正常的，然后我就写了个verify，看看每个Data中的数据有错误没
	//有错就打False，可就是没错，这就有点奇怪了，我将线程增加到了4个，class增加到1000，还是没错，让我有点犹豫是不是数据竞争不容易再现，然后我突然意识到
	//我所verify的指针应该都是正确的指针，当然打不出False了，于是我就用一个全局变量统计了调用次数，果然，4个线程1000个类，结果应该是调用了4000次。统计的次数有时就会少几个，那几个就是没赋值成功的指针了。
	
	//接下来再试试call_once。
	cnt = 0;
	vector<SomeObject> T(10000);
	thread T1(Test2, ref(T));
	thread T2(Test2, ref(T));
	thread T3(Test2, ref(T));
	thread T4(Test2, ref(T));
	T1.join();
	T2.join();
	T3.join();
	T4.join();
	cout << cnt;
	//....结果是call_once调用的次数比双重锁还少...琢磨了半天我意识到每次调用verify,全局变量加1的时候，并发写操作会导致覆盖啊，所以就在加1前面加了个锁，果然调用次数多了很多。
	//而且变成了call_once比双锁多，这意味着call_once的速度要比双锁快（这也是自然）。我把类量增至10000了，但还是总是与40000差几个,到底差在哪了呢？

}
