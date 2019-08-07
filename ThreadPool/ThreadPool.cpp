#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <thread>
#include <chrono>
#include "ThreadPool.hpp"

using namespace std;
int main()
{
	ThreadPool pool(4);
	vector<future<string>> results;
	for (int i = 0; i < 8; ++i)
	{
		results.emplace_back(
			pool.enqueue([i] {
			printf("hello %d\n", i);
			this_thread::sleep_for(chrono::seconds(i));
			printf("done %d\n", i);
			return string("---thread ") + to_string(i) + string(" finished.---");
		})
		);
	}
	for (auto && result : results)
		cout << result.get() << endl;
}
