#include "pch.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <map>
#include <iterator>
#include <vector>
using namespace std;

class someBigObject
{
	map<int, string> mIdName;
	mutex m1,m2;
public:
	someBigObject(){}
	someBigObject(map<int,string>& _m)
	{
		mIdName = _m;
	}
	void setData(int id, string name)
	{
		lock_guard<mutex> l1(m1); 
		lock_guard<mutex> l2(m2);
		mIdName[id] = name;
	}
	void showAll()
	{
		lock_guard<mutex> l1(m1);
		for (map<int,string>::iterator i = mIdName.begin(); i!= mIdName.end();i++)
		{
			cout << i->first << " " << i->second<<"  ";
		}
	}
	void showName(int id)
	{
		lock_guard<mutex> l2(m2);
//		lock_guard<mutex> l1(m1);	锁m1于情于理都不应该，但要真傻到这么写了，三个线程很容易就会死锁
		string _name = mIdName[id];
		if (_name == "")
			cout << "Not Found" << endl;
		else
			cout << _name << endl;
	}
	someBigObject& operator=(const someBigObject& other)
	{
		this->mIdName = other.mIdName;
		return *this;
	}
	someBigObject(someBigObject& other):mIdName(other.mIdName)
	{

	}
};
void swap(someBigObject& lhs, someBigObject& rhs)
{
	someBigObject temp = lhs;
	lhs = rhs;
	rhs = temp;
}
class avoidDeadclock:public someBigObject
{
	someBigObject someDetail;
	mutex m;
public:
	avoidDeadclock(someBigObject& sd):someDetail(sd)
	{}
	void showAll()
	{
		someDetail.showAll();
	}
	friend void safeSwap(avoidDeadclock& lhs, avoidDeadclock& rhs);
	friend void oops(avoidDeadclock& lhs, avoidDeadclock& rhs);

};
void safeSwap(avoidDeadclock& lhs, avoidDeadclock& rhs)
{
	lock(lhs.m, rhs.m);
	lock_guard<mutex> lockA(lhs.m, adopt_lock);
	lock_guard<mutex> lockB(rhs.m, adopt_lock);
	swap(lhs.someDetail, rhs.someDetail);
}
void oops(avoidDeadclock& lhs, avoidDeadclock& rhs)
{
	lock_guard<mutex> lockA(lhs.m);
	lock_guard<mutex> lockB(rhs.m);
	swap(lhs.someDetail, rhs.someDetail);
}
/*目前真没想出来怎么测试，实话说没有太看懂书上是什么意思，过些天再回头试试吧。就我目前的理解来说，死锁发生的情况在这个例子中是这样的：两个someBigObject实例，每个实例都
在多个线程中不停setData,showData，那这就得保证一个线程setdata的时候另一个不能刚好show,也不能相反，所以需要互斥元，someBigObject中为了保证showAll与showName可以并行我用
了两个互斥元，也不知道这样对不对，其实这时就已经可以直接运用lock锁两个互斥元了要不一不小心就死锁(L36)，而书中写的swap是想阐述一个问题就是能保证lock锁用对的情况下依然保
证不了能避免死锁，它叙述的一种情况：在多线程条件下，一个类中的两个实例想要交换数据，这得保证交换的时候数据不能被修改，也就是得把两个实例中的数据都锁住，如果这个object
包括很多很多东西，那最佳办法是新建一个专门为了交换或者说为了避免死锁的类，类中声明一个互斥元就行了，锁它一个等于锁了整个类，之后交换这块如果还是按顺序锁的话，交换完之后
也得按顺序解锁，可是交换之后先锁的变成后解锁，后锁的变成先解锁，这就死锁了！所以lock同时上锁无谓顺序这点就显得至关重要。
好像应该大概就是这么个意思吧。我可真是个小聪明鬼，好像给自己解释明白了。
那我该怎么重现死锁方式呢？
一个线程在顺序上锁的情况下进行swap交换应该都能重现吧

我错了，死锁是得有两个线程，并且正好同时交换两个相同的实例，比如swap(Aa,Ba)和swap(Ba,Aa)并发了，这样的话前者锁住了Aa的锁需要Ba没锁，后者相反，造成死锁。
*/
int main()
{
	someBigObject A,B;
	for (int i = 0; i < 100; i++)
	{
		A.setData(i + 1, to_string(i + 1));
		B.setData(i + 100, to_string(i + 100));
	}
	avoidDeadclock Aa(A);
	avoidDeadclock Ba(B);
	Aa.showAll();
	Ba.showAll();
	printf("\n");
	//vector<thread> threads(10);
	//for (int i = 0; i < 10; i++)
	//{
	//	threads[i] = thread(oops, ref(Aa), ref(Ba));
	//	threads[i + 1] = thread(oops, ref(Ba), ref(Aa));
	//	i++;
	//}
	//for (int i = 0; i < 10; i++)
	//{
	//	threads[i].join();
	//}                               死锁！
	vector<thread> threads(10);
	for (int i = 0; i < 10; i++)
	{
		threads[i] = thread(safeSwap, ref(Aa), ref(Ba));
		if (i == 8)
			break;
		threads[i + 1] = thread(safeSwap, ref(Ba), ref(Aa));  //我刚才i的值写错了，然后发现vector存thread越界不会报错，好像就是单纯的把越界的语句忽略了。
		i++;
	}
	for (int i = 0; i < 9; i++)
	{
		threads[i].join();
	}
	Aa.showAll();
	Ba.showAll();
	


}
