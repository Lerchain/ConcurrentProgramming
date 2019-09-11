#include "pch.h"
#include <iostream>
#include <thread>
#include <numeric>
#include <iterator>
#include <vector>
#include <future>
using namespace std;

class ThreadsGuard
{
	vector<thread>& threads;
public:
	explicit ThreadsGuard(vector<thread>& t):threads(t){}

	~ThreadsGuard()
	{
		for(int i = 0; i < threads.size(); i++)
		{
			if (threads[i].joinable())
				threads[i].join();
		}
	}
};

template<typename T, typename Iter>
T accumulateBlock(Iter first, Iter last)
{
	return accumulate(first, last,T());
}

template<typename T, typename Iter>
T parallelAccumulate(Iter first, Iter last,T init)
{
	int const len = distance(first, last);
	int const hardware = _Thrd_hardware_concurrency();
	int const minPerthread = 100;
	int const maxThread = (minPerthread + len - 1) / minPerthread;
	int const numThread = min(hardware ? hardware : 2, maxThread);
	int const blockSize = len / numThread;
	vector <future<T>> futures(numThread - 1);	//之所以都-1是因为主线程也算一个线程。
	vector<thread> threads(numThread-1);
	ThreadsGuard tg(threads);
	Iter blockStart = first , blockEnd = first;
	for (int i = 0; i < numThread - 1; i++)
	{
		blockEnd = blockStart;
		advance(blockEnd, blockSize);	
		packaged_task<T(Iter,Iter)> task(accumulateBlock<T,Iter>);
		futures[i] = task.get_future();
		threads[i] = thread(move(task), blockStart, blockEnd);
		blockStart = blockEnd;
	}
	T last_result = accumulateBlock<T,Iter>(blockStart, last);
	T result = init;
	for (int i = 0; i < (numThread - 1); i++)
	{
		result += futures[i].get();
	}
	result += last_result;
	return result;
}
int main()
{
	vector<int> v(1000, 1);
	int res = parallelAccumulate<int,vector<int>::iterator>(v.begin(), v.end(), 0);
	cout << res;
}
