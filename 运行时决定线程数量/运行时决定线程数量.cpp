#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <iterator>
using namespace std;

template<typename Iterator, typename T>
struct Accumulate_block
{
	void operator()(Iterator begin, Iterator end, T& res)
	{
		res = accumulate(begin, end, res);
	}
};


template<typename Iterator, typename T>
T parallel_accumulate(Iterator begin, Iterator end, T ini)
{
	unsigned long const len = distance(begin, end);
	if (!len)
		return ini;
	unsigned long const hardware_threads = _Thrd_hardware_concurrency();
	cout << "核心数量" << hardware_threads<<endl;
	unsigned long const num_threads = hardware_threads ? hardware_threads : 2;
	unsigned long const block_size = len / num_threads;
	Iterator block_start = begin;
	vector<T> res(num_threads);
	vector<thread> threads(num_threads-1);
	for (int i = 0; i < num_threads-1; i++)
	{
		Iterator block_end = block_start;
		advance(block_end, block_size);
		threads[i] = thread(Accumulate_block<Iterator, T>(), block_start, block_end, ref(res[i]));
		block_start = block_end;
	}
	thread t(Accumulate_block<Iterator, T>(), block_start, end, ref(res[num_threads - 1]));
	for (int i = 0; i < num_threads - 1; i++)
		threads[i].join();
	t.join();
	return accumulate(res.begin(), res.end(),ini);
}


int main()
{
	vector<int> test(100);
	for (int i = 0; i < 100; i++)
	{
		test[i] = i + 1;
	}
	int res1 = parallel_accumulate(test.begin(), test.end(), 0);
	cout << res1 << endl;
	vector<string> test2(100);
	for (int i = 0; i < 100; i++)
	{
		test2[i] = to_string(i);
	}
	string res2 = parallel_accumulate(test2.begin(),test2.end(),static_cast<string>(""));
	cout << res2 << endl;

}
