#include "pch.h"
#include <iostream>
#include <thread>
#include <string>

using namespace std;

void doSomething(thread::id _id)
{
	if (this_thread::get_id() == _id)
	{
		cout << "I am doing something only in main thread" << endl;
	}
	cout << "I am doing something" << endl;
}
int main()
{
	thread::id master_id;
	master_id = this_thread::get_id();
	cout << "Master id is "<<master_id<<endl;
	thread t(doSomething,ref(master_id));
	t.join();
	if (t.get_id() != master_id)
		cout << "t thread id " << t.get_id() << " is not equal to master_id"<< endl;
	doSomething(master_id);
}

