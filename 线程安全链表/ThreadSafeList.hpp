#include <iostream>
#include <mutex>
using namespace std;

template<typename T>
class ThreadSafeList
{
	struct node
	{
		shared_ptr<T> spVal;
		unique_ptr<node> unpNext;
		node(T _val):spVal(make_shared<T>(_val))
		{}
		node()
		{}
		mutex m;//要细粒度的锁上，能尽可能减小锁住的区域，增大并发量。
	};
	//mutex headMutex;
	node head;
public:
	ThreadSafeList<T>(){}
	ThreadSafeList<T>(const ThreadSafeList<T>& other) = delete;
	ThreadSafeList<T>& operator=(ThreadSafeList<T> const& other) = delete;
		
	void pushFront(T& val)
	{
		//shared_ptr<T> newshVal = make_shared<T>(val);
		unique_ptr<node> newNode(new node(val));
		lock_guard<mutex> lk(head.m);
		newNode->unpNext = move(head.unpNext);
		head.unpNext = move(newNode);
	}

	template<typename Function>
	void forEach(Function f)
	{
		node* cur = &head;
		unique_lock<mutex> lk(head.m);
		while (node* n = cur->unpNext.get())	//这哪错了。
		{
			unique_lock<mutex> nlk(n->m);
			lk.unlock();
			f(*n->spVal);
			cur = n;
			lk = move(nlk);
		}
	}
	template<typename P>
	shared_ptr<T> findFisrstIf(P p)
	{
		node* cur = &head;
		unique_lock<mutex> lk(head.m);
		while (node* const n = cur->unpNext.get())
		{
			unique_lock<mutex> nlk(n->m);
			lk.unlock(); 
			if (p(*n->spVal))
			{
				return n->spVal;
			}
			cur = n;
			lk = move(nlk);
		}
		return shared_ptr<T>();
	}

	template<typename P>
	void removeif(P p)
	{
		node* cur = &head;
		unique_lock<mutex> lk(head.m);
		while (node* const n = cur->unpNext.get())
		{
			unique_lock<mutex> nlk(n->m);
			if (p(*n->spVal))	//如果符合
			{
				unique_ptr<node> oldNext = move(cur->unpNext); 
				cur->unpNext = move(n->unpNext);	
				cur = cur->unpNext.get();
				nlk.unlock();
			}
			else
			{
				lk.unlock();
				cur = n;
				lk = move(nlk);
			}
		}
	}
};