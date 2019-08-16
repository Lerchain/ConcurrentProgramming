#include "pch.h"
#include <iostream>
#include <thread>
#include <atomic>

using namespace std;

class Test
{
	atomic<bool> x= false, y = false;
	atomic<int> z = 0;

public:
	
	void writeX()
	{
		x.store(true, memory_order_seq_cst);
	}


	void writeY()
	{
		y.store(true, memory_order_seq_cst);
	}


	void readXthenY()
	{
		while (x.load() != true);
		if (y.load() == true)
			z++;
	}


	void readYthenX()
	{
		while (y.load() != true);
		if (x.load() == true)
			z++;
	}
	void verify()
	{
		if (z == 0)
			cout << "Impossible";
		
		cout << "z = "<<z<<". Definitely >0";
	}
};
int main()
{
	Test t;
	thread t1(&Test::writeX,&t);
	thread t2(&Test::writeY,&t);
	thread t3(&Test::readXthenY,&t);
	thread t4(&Test::readYthenX,&t);
	t.verify();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}

