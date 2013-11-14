#include "menu.h"
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
using namespace runner;

//need to implement checks for extensions,
//as well as spaces in file names.

//also need to remove windows.h dependencies

//Input

namespace runner
{
	std::ostream& operator<<(std::ostream& stream, const Menu& menu)
	{
		if (menu.isUpdated)
		{
			system("CLS");
			for (int i = 0; i < menu.size; i++)
			{
				stream << ((i == menu.cursor) ? ">":" ");
				stream << menu.options[i] << std::endl;
			}
		}
		return stream;
	}
}

int Input::get() const
{
	if (a == -1)
		return 0;
	if (b != 0)
		return -b;
	return a;
}

void Input::set()
{
	if (kbhit())
	{
		a = _getch();
		if (a == 0 or a == 224)
			b = _getch();
		else
			b = 0;
	}
	else
		a = -1;
}

void Input::waitfor()
{
	a = _getch();
	if (a == 0 or a == 224)
		b = _getch();
	else
		b = 0;
}

void Input::wait() const
{
	_getch();
}

//Menu

void Menu::getInput()
{
	input.set();
}

//cMenu

void cMenu::execute()
{
	isUpdated = true;
	if (input.get() == -72 and cursor > 0)
		cursor--;
	else if (input.get() == -80 and cursor < size - 1)
		cursor++;
	else if (input.get() == 0)
		isUpdated = false;
}

cMenu::cMenu()
{
	changeto = "";
	size = 0;
}

//projectMenu

void projectMenu::execute()
{
	if (input.get() == 13)
	{
		if (cursor == size-1)
			exit(0);
		else if (cursor == size-2)
		{
			std::cout << "Running...\n";
			system(commands[cursor].c_str());
			std::cout << "Finished.\nPress any key to continue.\n";
			input.wait();
		}
		else
		{
			std::cout << "Compiling...\n";
			system(commands[cursor].c_str());
			std::cout << "Finished.\nPress any key to continue.\n";
			input.wait();
		}
	}
	if (input.get() == 8)
		changeto = "main";
	cMenu::execute();
}

void projectMenu::setup(std::string folder) const
{
	system("start notepad++.exe");
	std::string filename = folder + "\\main.cpp";
	std::ifstream mfile(filename.c_str());
	if (!mfile)
	{
		std::ofstream ofile(filename.c_str());
		ofile << "#include <iostream>\n";
		for (int i = 0; i < hfiles.size(); i++)
			ofile << "#include \"" << hfiles[i] << ".h\"\n";
		ofile << "using namespace std;\n\nmain()\n{\n\t\n}";
		ofile.close();
	}
	system(filename.c_str());
	mfile.close();
	for (int i = 0; i < cppfiles.size(); i++)
	{
		filename = folder + "\\" + cppfiles[i] + ".cpp";
		std::ifstream cppfile(filename.c_str());
		if (!cppfile)
		{
			std::ofstream ofile(filename.c_str());
			ofile << "#include \"" << cppfiles[i] << ".h\"\n";
			ofile.close();
		}
		system(filename.c_str());
	}
	for (int i = 0; i < hfiles.size(); i++)
	{
		filename = folder + "\\" + hfiles[i] + ".h";
		std::ifstream hfile(filename.c_str());
		if (!hfile)
		{
			std::ofstream ofile(filename.c_str());
			ofile.close();
		}
		system(filename.c_str());
	}
}

projectMenu::projectMenu(tag proj): cMenu()
{
	std::string name, mainname, list;
	const tag* child = proj.next_child();
	bool multiple = false;
	while (child != NULL)
	{
		std::string tagname = child->get_name();
		if (tagname == "name")
			name = child->get_content();
		if (tagname == "main")
		{
			mainname = child->get_content();
			options.push_back("compile main");
			commands.push_back("g++ -c " + name + "\\" + child->get_content() + ".cpp -o "
				+ name + "\\" + child->get_content() + ".o");
			list += name + "\\" + child->get_content() + ".o ";
		}
		if (tagname == "cpp")
		{
			options.push_back("compile " + child->get_content());
			cppfiles.push_back(child->get_content());
			hfiles.push_back(child->get_content());
			commands.push_back("g++ -c " + name + "\\" + child->get_content() + ".cpp -o "
				+ name + "\\" + child->get_content() + ".o");
			list += name + "\\" + child->get_content() + ".o ";
			multiple = true;
		}
		if (tagname == "h")
			hfiles.push_back(child->get_content());
		child = proj.next_child();
	}
	if (multiple)
	{
		options.push_back("link executable");
		commands.push_back("g++ -o " + name + ".exe " + list);
	}
	else
	{
		options.push_back("compile");
		commands.push_back("g++ -o " + name + ".exe " + name + "\\main.cpp ");
	}
	options.push_back("run program");
	commands.push_back(name + ".exe");
	options.push_back("exit");
	size = options.size();
	setup(name);
	isUpdated = true;
}

//startMenu

void startMenu::execute()
{
	cMenu::execute();
	if (input.get() == 13)
	{
		if (cursor == size-1)
			changeto = newProject();
		else
			changeto = options[cursor];
	}
	if (input.get() == -83 and cursor != size-1)
		deleteProject();
}

startMenu::startMenu(std::string file): cMenu()
{
	configfile = file;
	checkfor(file);
	std::ifstream ifile(file.c_str());
	if (ifile)
	{
		do
		{
			tag curTag;
			curTag.read(ifile);
			if (curTag.get_name() == "project")
			{
				std::string option;
				while (option == "")
				{
					const tag* child = curTag.next_child();
					if (child->get_name() == "name")
						option = child->get_attribute().length()==0 ? child->get_content() : child->get_attribute();
				}
				options.push_back(option);
			}
		} while (ifile.good());
		options.push_back("New Project");
		size = options.size();
		isUpdated = true;
	}
	ifile.close();
}
	
std::string startMenu::newProject() const
{
	std::string name, tmp;
	tag* file;
	int pairs, files;
	tagmaker manager;
	std::cout << "Name for the project?\n";
	getline(std::cin, name);
	bool exists = false;
	for (int i = 0; i < options.size(); i++)
		if (name == options[i])
			exists = true;
	if (exists)
	{
		std::cout << "A project named \"" << name << "\" already exists.\n";
		return "";
	}
	std::string mkdir = "mkdir " + name;
	system(mkdir.c_str());
	std::cout << "How many cpp/h pairs?\n";
	std::cin >> pairs;
	std::cout << "How many lone h files?\n";
	std::cin >> files;
	getline(std::cin, tmp);
	file = new tag[pairs + files + 2];
	manager.manage(file[0]);
	manager.set_name("name");
	manager.set_content(name);
	manager.manage(file[1]);
	manager.set_name("main");
	manager.set_content(name);
	for (int i = 0; i < pairs; i++) 
	{
		manager.manage(file[i+2]);
		std::cout << "Name pair " << i+1 << ".\n";
		getline(std::cin, tmp);
		manager.set_name("cpp");
		manager.set_content(tmp);
	}
	for (int i = 0; i < files; i++) 
	{
		manager.manage(file[i + pairs + 2]);
		std::cout << "Name header " << i+1 << ".\n";
		getline(std::cin, tmp);
		manager.set_name("h");
		manager.set_content(tmp);
	}
	addProject(file, pairs+files+2);
	return name;
}

void startMenu::addProject(tag* child, int files) const
{
	SetFileAttributes(configfile.c_str(), FILE_ATTRIBUTE_NORMAL);
	std::ofstream ofile(configfile.c_str(), std::ofstream::app);
	tag* newproject = new tag;
	tagmaker manager(newproject);
	manager.set_name("project");
	for (int i = 0; i < files; i++)
		manager.add_child(child[i]);
	manager.prepare();
	newproject->write(ofile);
	delete newproject;
	ofile.close();
	SetFileAttributes(configfile.c_str(), FILE_ATTRIBUTE_HIDDEN);
}

void startMenu::deleteProject()
{
	std::cout << "Are you sure you want to delete? (y/n)";
	input.waitfor();
	int in = input.get();
	if (in == 121)
	{
		SetFileAttributes(configfile.c_str(), FILE_ATTRIBUTE_NORMAL);
		std::ifstream ifile(configfile.c_str());
		std::vector<tag> tags;
		if (ifile)
		{
			do {
				tag curTag;
				curTag.read(ifile);
				if (curTag.get_name() == "project")
					tags.push_back(curTag);
			} while (ifile.good());
		}
		ifile.close();
		std::ofstream ofile(configfile.c_str());
		for (int i = 0; i < tags.size(); i++)
		{
			if (i == cursor)
			{
				std:: string rem = "RD /S /Q " + options[i];
				system(rem.c_str());
				for (int j = 0; j < size; j++)
					if (j == cursor)
						options.erase(options.begin()+j);
			}
			else
				tags[i].write(ofile);
		}
		ofile.close();
		size--; 
		isUpdated = true;
		SetFileAttributes(configfile.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}
}

std::string startMenu::checkfor(std::string file) const
{
	std::ifstream ifile(file.c_str());
	if (!ifile)
	{
		std::ofstream ofile(file.c_str());
		ofile.close();
		ifile.close();
		SetFileAttributes(file.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}
	return file;
}

//Loader

Loader* Loader::instance = NULL;

Loader* Loader::get(std::string filename)
{
	if (instance == NULL)
		instance = new Loader(filename);
	return instance;
}

void Loader::load()
{
	if (curMenu == NULL)
	{
		curMenu = new startMenu(file);
	}
	else if (curMenu->changeto == "main")
	{
		delete curMenu;
		curMenu = new startMenu(file);
	}
	else if (curMenu->changeto != "")
	{
		std::ifstream ifile(file.c_str());
		tag curTag;
		if (ifile)
		{
			bool found = false;
			while (!found)
			{
				curTag.clear();
				curTag.read(ifile);
				if (curTag.get_name() == "project")
				{
					const tag* child = curTag.next_child();
					while (!found and child != NULL)
					{
						if (child->get_name() == "name" and child->get_content() == curMenu->changeto)
							found = true;
						child = curTag.next_child();
					}
				}
			}
		}
		tagmaker manager(curTag);
		manager.prepare();
		ifile.close();
		delete curMenu;
		curMenu = new projectMenu(curTag);
	}
}

Loader::Loader(std::string filename)
{
	file = filename;
	curMenu = NULL;
}

Loader::~Loader()
{
	if (curMenu != NULL)
		delete curMenu;
}