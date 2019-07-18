#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
#include <mutex>
using namespace std;

class someBigObject 
{
private:
	int id[1000];
	string name[1000];
public:
	void setData(int id,int num, string name)
	{
		this->id[num] = id;
		this->name[num] = name;
	}
	friend void swap(someBigObject& lhs, someBigObject& rhs);
	void showall()
	{
		for (int i = 0; i < 100; i++)
		{
			cout << id[i] << " ";
		}
	}
};
void swap(someBigObject& lhs, someBigObject& rhs)
{
	swap(lhs.id, rhs.id);
	swap(lhs.name, rhs.name);
}
class doSomething
{
private:
	mutex m;
public:
	someBigObject somedetail;
	doSomething(const someBigObject& x) : somedetail(x) {}
	
	friend void swap(doSomething& lhs, doSomething& rhs)
	{
		unique_lock<mutex> lh(lhs.m, defer_lock);
		unique_lock<mutex> lr(rhs.m, defer_lock);
		lock(lh, lr);
		swap(lhs.somedetail, rhs.somedetail);
	}
	friend void oops(doSomething& lhs, doSomething& rhs)
	{
		swap(lhs.somedetail, rhs.somedetail);
	}
};
void change(doSomething& dA, doSomething& dB)
{
	for (int i = 0; i < 100; i++)
	{
		swap(dA, dB);
	}
}
void oopsChange(doSomething& dA, doSomething& dB)
{
	for (int i = 0; i < 100; i++)
	{
		oops(dA, dB);
	}
}

void prepairData()
{
	cout <<endl<< "Hello" << endl;
}
mutex m;
unique_lock<mutex> getlock()
{
	unique_lock<mutex> lk(m);
	prepairData();
	return lk;
}
void processData()
{
	unique_lock<mutex> lc(getlock());
	cout << "unique lock ownership moved" << endl;
	lc.unlock();
	cout << "unique lock unlocked" << endl;
}
int main()
{
	someBigObject A, B;
	for (int i = 0; i < 1000; i++)
	{
		A.setData(i + 1, i, to_string(i));
		B.setData(-i - 1, i, to_string(-i));
	}
	doSomething dA(A), dB(B);
	//A.showall();
	//B.showall();
//	dA.somedetail.showall();
//	dB.somedetail.showall();
	thread t1(change, ref(dA), ref(dB));
	thread t2(change, ref(dA), ref(dB));
	t1.join();
	t2.join();
	//A.showall();
	//B.showall();
	dA.somedetail.showall();
	dB.somedetail.showall();
	cout << endl;
	thread t3(oopsChange, ref(dA), ref(dB));
	thread t4(oopsChange, ref(dA), ref(dB));
	t3.join();
	t4.join();
	//A.showall();
	//B.showall();
	dA.somedetail.showall();
	dB.somedetail.showall();
	thread t5(processData);
	t5.join();
}

