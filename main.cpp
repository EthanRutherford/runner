#include <iostream>
#include <windows.h>
#include "menu.h"
using namespace std;
using namespace runner;

int main()
{
	system("title Runner version: 8.0");
	Loader* menuLoader = Loader::get("projects.cfg");
	while (true)
	{
		menuLoader->load();
		cout << *menuLoader->menu();
		menuLoader->menu()->getInput();
		menuLoader->menu()->execute();
	}
}