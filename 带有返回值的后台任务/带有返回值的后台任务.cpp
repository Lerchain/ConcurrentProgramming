#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <future>
#include <thread>
int calculate()
{
	std::cout << "Calculate begin()" << std::endl;;
	int res = 0;
	for (int i = 0; i < 1000; i++)
	{
		res += i;
	}
	std::cout << "Calculate finished" <<res<< std::endl;
	return res;
}

int main()
{
	std::future<int> p = std::async(calculate);
	Sleep(3000);  //看看p运没运作
	std::cout << "main thread" << std::endl;
	std::cout<<p.get()<<std::endl;

	std::future<int> q = std::async(std::launch::deferred, calculate); //调用时再调用。
	std::cout << "Second test" << std::endl;  
	Sleep(3000);
	std::cout << q.get();

	std::future<int> r = std::async(std::launch::async, calculate); //调用时必须开启单独的线程
	std::cout << "Third test" << std::endl;
	Sleep(3000);
	std::cout << r.get(); 

	//多线程调试时，要将断点设在线程执行的方法内。我把断点设在了future上，调试半天，怎么都是主线程。
}

