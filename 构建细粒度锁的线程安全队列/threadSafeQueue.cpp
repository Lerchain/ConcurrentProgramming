#include "pch.h"
#include "threadSafeQueue.h"

template<typename T>
bool threadSafeQueue<T>::empty()
{
	lock_guard<mutex> lk(headMutex);
	if (head.get() == getTail())
		return true;
	return false;
}

template<typename T>
void threadSafeQueue<T>::push(T val)
{
	//tail->val = make_shared<T>(move(val);//为了尽可能的锁住少的区域扩展此代码。更重要的原因是为了保护程序特殊情况下不会崩溃。
	shared_ptr<T> newVal(make_shared<T>(val));
	unique_ptr<node> nxt(new node());
	{
		lock_guard<mutex> lk(tailMutex);
		tail->val = newVal;			//奇了怪了这里为什么会出异常。成员初始化顺序啊啊啊啊
	//	tail->val = make_shared<T>(val);
		node* newTail = nxt.get();
		tail->next = move(nxt);		//这里要注意nxt已经move了
		tail = newTail;
	}
	cv.notify_one();
}

template<typename T>
typename threadSafeQueue<T>::node* threadSafeQueue<T>::getTail()	//这里需要加typename，否则识别不出node是个类型。
{
	lock_guard<mutex> lk(tailMutex);
	return tail;
}

template<typename T> //有点麻烦，为什么类外每个函数都需要加上这个。
shared_ptr<T> threadSafeQueue<T>::tryPop()
{
	lock_guard<mutex> lk(headMutex);
	if (head.get() == getTail())
		return shared_ptr<T>();
	shared_ptr<T> res = head->val;
	head = move(head->next);
	return res;
}

template<typename T>
bool threadSafeQueue<T>::tryPop(T& val)
{
	lock_guard<mutex> lk(headMutex);
	if (head.get() == getTail())
		return false;
	val = head->val;
	head = move(head->next);
	return true;
}

template<typename T>
shared_ptr<T> threadSafeQueue<T>::waitAndPop()
{
	cv.wait(headMutex, [&] {return head.get() != getTail(); });
	shared_ptr<T> res = make_shared<T>(head->val);
	head = move(head->next);
	return res;
}

template<typename T>
void threadSafeQueue<T>::waitAndPop(T& val)
{
	unique_lock<mutex> lk(headMutex);
	cv.wait(lk, [&] {return head.get() != getTail(); });
	//shared_ptr<T> p = head->val;
	//val = *p;
	val = move(*head->val);
	head = move(head->next);
}