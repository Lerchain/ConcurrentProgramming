#include "pch.h"
#include <iostream>
#include <list>
#include <string>
#include <algorithm>
#include <thread>
#include <future>

using namespace std;

list<int> parrallel_quick_sort(list<int> input)//参数不能用引用
{
	if (input.empty())
		return {};
	int key = input.front();
	list<int> res;
	res.splice(res.begin(), input, input.begin());
	auto it = stable_partition(input.begin(), input.end(), [=](int x) {return x < key; });
	list<int> lower_part,higher_part;
	lower_part.splice(lower_part.begin(),input,input.begin(),it);
	higher_part.splice(higher_part.begin(), input, input.begin(),input.end());
	auto new_lower(async(&parrallel_quick_sort, move(lower_part)));
	higher_part = parrallel_quick_sort(higher_part);
	res.splice(res.begin(), new_lower.get());
	res.splice(res.end(), higher_part);
	return res;
}
int main()
{
	list<int> l{ 10,8,7,6,5,3,1,2,4,9};
	l = parrallel_quick_sort(l);
	for (auto x : l)
		cout << x;
}
