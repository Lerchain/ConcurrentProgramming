
#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
using std::cout;
using std::cin;
void open_document_and_display(std::string const& filename)
{
	cout << "Hello " << filename;
}
char done_editing()
{
	char input;
	cin >> input;
	return input;
}
std::string get_file_from_user()
{
	std::string new_name;
	cin >> new_name;
	return new_name;
}
void keep_edit_something(std::string const filename)
{
	cout << filename << " is editing";
}
void edit_document(std::string const& filename)
{
	open_document_and_display(filename);
	char input = 'q';
	do
	{
		input = done_editing();
		if (input == 'o')
		{
			std::string const new_name = get_file_from_user();
			std::thread t(edit_document,new_name);
			t.detach();
		}
		else
		{
			keep_edit_something(filename);
		}
	} while (input!='q');
}
int main()
{
	edit_document("Lerchain");
}

