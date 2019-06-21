#include "pch.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <string>

//刚才已经照书敲了一遍，打算自己再写一遍试试
using namespace std;

class HierarchyMutex
{
	mutex internalMutex;
	unsigned long const hierarchyMutexValue;
	unsigned long previousMutexValue;
	static thread_local unsigned long thisMutexValue;
	
	
	void checkViolate()
	{
		if (thisMutexValue <= hierarchyMutexValue)
			throw logic_error ("Mutex value violated");
	}
	
	void update()
	{
		previousMutexValue = thisMutexValue;
		thisMutexValue = hierarchyMutexValue;
	}
public:
	explicit HierarchyMutex(unsigned long val): hierarchyMutexValue(val), previousMutexValue(0)
	{}
	void lock()
	{
		checkViolate();
		internalMutex.lock();
		update();
	}
	
	void unlock()
	{
		thisMutexValue = previousMutexValue;
		internalMutex.unlock();
	}
	
	bool tryLock()             //trylock的作用是当其他线程已经获取锁时，当前线程直接返回false而不是继续等待。
	{
		checkViolate();
		if (!internalMutex.try_lock())
		{
			return false;
		}
		update();
		internalMutex.lock();
		return true;
	}
};
thread_local unsigned long HierarchyMutex::thisMutexValue = ULLONG_MAX;

HierarchyMutex highLevelMutex(1000);
HierarchyMutex lowLevelMutex(500);

void threadA()
{
	lock_guard<HierarchyMutex> lk(highLevelMutex);
	cout << "I am doing something under high level mutex protection" << endl;
	lock_guard<HierarchyMutex> lk2(lowLevelMutex);
	cout << "I am doing something under low level mutex protection" << endl;
}
void threadB()
{
	lock_guard<HierarchyMutex> lk(lowLevelMutex);
	cout << "I am doing something under low level mutex protection" << endl;
	lock_guard<HierarchyMutex> lk2(highLevelMutex);
	cout << "Oops, exception came out" << endl;
}
int main()
{
	thread tA(threadA);
//	thread tB(threadB);
	tA.join();
//	tB.join();
}

//success