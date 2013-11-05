#include <iostream>
#include "menu.h"
using namespace std;

int main()
{
	string file = "";
	startMenu Smenu("projects.cfg");
	while (true)
	{
		if (file == "")
		{
			cout << Smenu;
			Smenu.getInput();
			file = Smenu.execute();
		}
		else
		{
			projectMenu Pmenu(file);
			while (!Pmenu.back())
			{
				cout << Pmenu;
				Pmenu.getInput();
				Pmenu.execute();
			}
			file = "";
		}
	}
}