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
	//tail->val = make_shared<T>(move(val);//Ϊ�˾����ܵ���ס�ٵ�������չ�˴��롣����Ҫ��ԭ����Ϊ�˱���������������²��������
	shared_ptr<T> newVal(make_shared<T>(val));
	unique_ptr<node> nxt(new node());
	{
		lock_guard<mutex> lk(tailMutex);
		tail->val = newVal;			//���˹�������Ϊʲô����쳣����Ա��ʼ��˳�򰡰�����
	//	tail->val = make_shared<T>(val);
		node* newTail = nxt.get();
		tail->next = move(nxt);		//����Ҫע��nxt�Ѿ�move��
		tail = newTail;
	}
	cv.notify_one();
}

template<typename T>
typename threadSafeQueue<T>::node* threadSafeQueue<T>::getTail()	//������Ҫ��typename������ʶ�𲻳�node�Ǹ����͡�
{
	lock_guard<mutex> lk(tailMutex);
	return tail;
}

template<typename T> //�е��鷳��Ϊʲô����ÿ����������Ҫ���������
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