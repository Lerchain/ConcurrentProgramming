#include "pch.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "threadSafeQueue.h"
#include "threadSafeQueue.cpp"
using namespace std;

template<typename T>
class SingleThreadQueue
{
	struct node
	{
		T data;
		unique_ptr<node> next;
		node(T _data):data(move(_data))
		{}
	};
	unique_ptr<node> head;
	node* tail;
	//unique_ptr<node> tail;
public:
	SingleThreadQueue()
	{}
	
	SingleThreadQueue(const SingleThreadQueue& other) = delete;
	
	SingleThreadQueue& operator=(const SingleThreadQueue& other) = delete;

	shared_ptr<T> try_pop()
	{
		if (!head)
		{
		//	return nullptr;	//经测试，nullptr也行。
		//	return NULL; 甚至NULL都可以。
			return shared_ptr<T>();	
		}
		shared_ptr<T> const res(make_shared<T>(move(head->data)));
		unique_ptr<node> const old_head = move(head);	//如果去掉上一行的move还勉强可以，只是多费点空间，这个move就不行了，unique_ptr不能拷贝，只能转移。
		head = move(old_head->next);//同上
		return res;
	}
	void push(T newValue)	//以下注释部分皆为tail设为unique_ptr，最低级错误示范。
	{
		unique_ptr<node> p(new node(move(newValue)));
		node* const newTail = p.get();
		//unique_ptr<node> newTail(new node(move(newValue)));
		if (tail)
		{
			tail->next = move(p);
		//	tail->next = move(newTail);
		}
		else
		{
			head = move(p);
		//	node* const h = newTail.get();
		//	head = move(h);		//失败在这里，如果队列本为空，push后头指针与尾指针皆应指向相同元素，unique_ptr又怎会有两个指针指向相同对象。
		}
		tail = newTail;
//		tail = move(newTail);
	}
};

template<class T>
class QueueWithVirtualNode
{
private:
	struct node	
	{
		shared_ptr<T> data;
		unique_ptr<node> next;
	};
	unique_ptr<node> head;
	node* tail;
public:
	QueueWithVirtualNode():head(new node),tail(head.get()){}	//先创建个空节点
	QueueWithVirtualNode(const QueueWithVirtualNode& other) = delete;
	QueueWithVirtualNode& operator=(const QueueWithVirtualNode& other) = delete;

	shared_ptr<T> tryPop()
	{
		if (head.get() == tail)	//因为有空节点，所以这是队列为空的标志。
		{
			return shared_ptr<T>();
		}
		shared_ptr<T> const res(head->data);
		unique_ptr<node> oldHead = move(head);
		head = move(oldHead->next);
		return res;
	}
	void push(T newValue)
	{
		shared_ptr<T> newData(make_shared<T>(move(newValue)));	
		unique_ptr<node> p(new node);
		tail->data = newData;		//这么做都是为了在push中不引入对head的写操作。
		node* const newTail = p.get();
		tail->next = move(p);
		tail = newTail;
	}
};
void pushData(threadSafeQueue<int>& q)
{
	for (int i = 1; i < 100; i++)
	{
		q.push(i);
	}
}
void readData(threadSafeQueue<int>& q)
{
	for (int i = 0; i < 200; i++)
	{
		int val;
		q.waitAndPop(val);
		printf("%d ", val);
	}
}
int main()
{
	SingleThreadQueue<int> s;
	for (int i = 0; i < 10; i++)
	{
		s.push(i);
	}
	for (int i = 0; i < 11; i++)
	{
		shared_ptr<int> p = s.try_pop();
		if(p)
		cout << *p << endl;
	}
	QueueWithVirtualNode<int> vnq;
	for (int i = 0; i < 10; i++)
	{
		vnq.push(i);
	}
	for (int i = 0; i < 11; i++)
	{
		shared_ptr<int> p = vnq.tryPop();
		if (p)
			cout << *p << endl;
	}

	threadSafeQueue<int> q;
	/*thread t(pushData,ref(q));
	thread t2(pushData,ref(q));
	thread t3(readData,ref(q));*/
	atomic<bool> start = false;
	thread t([&] {while (!start.load()); for (int i = 0; i < 100; i++) q.push(i); });
	thread t2([&] {while (!start.load()); for (int i = 0; i < 100; i++) q.push(i + 100); });
	//thread t3([&]{for (int i = 0; i < 200; i++) { int t; q.waitAndPop(t); printf("%d ", t); } });
	int cnt = 0;
	thread t3([&]{
		while (!start.load());
		for (int i = 0; i < 200; i++)
		{
			shared_ptr<int> p;
			p = move(q.tryPop());
			if (p)
			{
				printf("%d ", *p);
				cnt++;
			}
		}
			if (cnt == 200)
				printf("True");
			else
				printf("False");
	});
	start = true;
	t.join();
	t2.join();
	t3.join();
	//for (int i = 1; i < 100; i++)
	//{
	//	q.push(i);
	//}
	//for (int i = 0; i < 100; i++)
	//{
	//	shared_ptr<int> p(move(q.tryPop()));
	//	if (p)
	//		cout << *p;
	//}
	
}