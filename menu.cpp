#include "menu.h"
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>

//need to implement checks for extensions,
//as well as spaces in file names.

//Input

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

//Menu

void Menu::getInput()
{
	input.set();
}

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

cMenu::cMenu(std::string file)
{
	std::ifstream ifile(file.c_str());
	if (ifile)
	{
		ifile >> size;
		ifile.close();
	}
	else
	{
		std::cout << "no such file\n";
		options = NULL;
	}
}

//projectMenu

void projectMenu::execute()
{
	if (input.get() == 13)
	{
		if (cursor == size-1)
			exit(0);
		else
			system(commands[cursor].c_str());
	}
	if (input.get() == 8)
		Back = true;
	cMenu::execute();
}

void projectMenu::setup(std::string*stuff) const
{
	std::string tmp = "";
	for (int i = 0; i < commands[size-3].size() - 4; i++)
		tmp += commands[size-3][i];
	for (int i = 1; i < size-3; i++)
	{
		std::string word = tmp + "\\" + stuff[i] + ".cpp";
		std::ifstream sfile(word.c_str());
		if (!sfile)
		{
			std::ofstream file(word.c_str());
			file << "#include \"" << stuff[i] << ".h\"";
			file.close();
		}
		sfile.close();
		word = tmp + "\\" + stuff[i] + ".h";
		std::ifstream hfile(word.c_str());
		if (!hfile)
		{
			std::ofstream file(word.c_str());
			file << "";
			file.close();
		}
		hfile.close();
	}
	std::string word = tmp + "\\" + stuff[0] + ".cpp";
	std::ifstream mfile(word.c_str());
	if (!mfile)
	{
		std::ofstream file(word.c_str());
		file << "#include <iostream>\n";
		for (int i = 1; i < size-3; i++)
			file << "#include \"" << stuff[i].c_str() <<".h\"\n";
		file << "using namespace std;\n\nint main()\n{\n\n}";
		file.close();
	}
	mfile.close();
	system("start notepad++.exe");
	for (int i = 0; i < size-2; i++)
	{
		std::string line = tmp + "\\" + stuff[i] + ".cpp";
		system(line.c_str());
		line = tmp + "\\" + stuff[i] + ".h";
		if (i > 0)
			system(line.c_str());
	}
}

projectMenu::projectMenu(std::string file): cMenu(file)
{
	std::ifstream ifile(file.c_str());
	if (ifile)
	{
		size += 2;
		options = new std::string[size];
		commands = new std::string[size-1];
		std::string* word;
		word = new std::string[size-2];
		getline(ifile, *word);
		for (int i = 0; i < size - 2; i++)
			getline(ifile, word[i]);
		for (int i = 0; i < size - 3; i++)
			options[i] = "compile " + word[i];
		options[size-3] = "link executable";
		options[size-2] = "run program";
		options[size-1] = "exit";
		for (int i = 0; i < size - 3; i++)
		{
			commands[i] = "g++ -c " + word[size-3] + "\\" + word[i] + ".cpp -o ";
			commands[i] += word[size-3] + "\\" + word[i] + ".o";
		}
		commands[size-3] = "g++ -o "+ word[size-3] + ".exe ";
		for (int i = 0; i <= size-3; i++)
			commands[size-3] += word[size-3] + "\\" + word[i] + ".o ";
		commands[size-3] = word[size-3] + ".exe";
		setup(word);
		delete [] word;
		ifile.close();
		isUpdated = true;
		Back = false;
	}
	else
		commands = NULL;
}

projectMenu::~projectMenu()
{
	if (options != NULL)
		delete [] options;
	if (commands != NULL)
		delete [] commands;
}

//startMenu

std::string startMenu::execute()
{
	std::string file = "";
	cMenu::execute();
	if (input.get() == 13)
	{
		if (cursor == size-1)
		{
			file = newProject();
			if (file != "")
				file += "\\project.cfg";			
		}
		else
			file = options[cursor] + "\\project.cfg";
	}
	if (input.get() == -83 and cursor != size-1)
		deleteProject();
	return file;
}

startMenu::startMenu(std::string file): cMenu((checkfor(file)))
{
	size++;
	options = new std::string[size];
	std::ifstream ifile(file.c_str());
	if (ifile)
	{
		std::string junk;
		getline(ifile, junk);
		for (int i = 0; i < size - 1; i++)
			getline(ifile, options[i]);
		options[size-1] = "New Project";
		ifile.close();
		isUpdated = true;
	}
}

startMenu::~startMenu()
{
	if (options != NULL)
		delete [] options;
}
	
std::string startMenu::newProject() const
{
	std::string* file, name, junk;
	std::cout << "Name for the project?\n";
	getline(std::cin, name);
	std::string mkdir = "mkdir " + name;
	system(mkdir.c_str());
	addProject(name);
	if (name == "")
		return name;
	std::cout << "How many additional source files for classes?\n";
	int files;
	std::cin >> files;
	getline(std::cin, junk);
	files += 2;
	file = new std::string[files];
	file[0] = "main";
	for (int i = 1; i < files-1; i++) 
	{
		std::cout << "Name file " << i << ".\n";
		getline(std::cin, file[i]);
	}
	file[files-1] = name;
	saveProject(name, file, files);
	return name;
}

void startMenu::saveProject(std::string project, std::string* file, int files) const
{
	std::string word = project + "\\project.cfg";
	std::ifstream ifile(word.c_str());
	std::ofstream ofile(word.c_str());
	ofile << files << std::endl;
	for (int i = 0; i < files; i++)
		ofile << file[i].c_str() << std::endl;
	ofile.close();
	ifile.close();
	SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_HIDDEN);
}

void startMenu::addProject(std::string& project) const
{
	std::string word = "projects.cfg";
	SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_NORMAL);
	std::ifstream ifile(word.c_str());
	std::string junk, line[size-1];
	getline(ifile, junk);
	for (int i = 0; i < size-1; i++)
		getline(ifile, line[i]);
	ifile.close();
	for (int i = 0; i < size-1; i++)
		if (line[i] == project)
		{
			SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_HIDDEN);
			project = "";
			system("pause");
			return;
		}
	std::ofstream ofile(word.c_str());
	ofile << size << std::endl;
	for (int i = 0; i < size-1; i++)
		ofile << line[i] << std::endl;
	ofile << project;
	ofile.close();
	SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_HIDDEN);
}

void startMenu::deleteProject()
{
	std::cout << "Are you sure you want to delete? (y/n)";
	input.waitfor();
	int in = input.get();
	if (in == 121)
	{
		std::string word = "projects.cfg";
		SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_NORMAL);
		std::ifstream ifile(word.c_str());
		std::string junk, line[size-1];
		getline(ifile, junk);
		for (int i = 0; i < size-1; i++)
			getline(ifile, line[i]);
		ifile.close();
		std::ofstream ofile(word.c_str());
		ofile << size-2 << std::endl;
		for (int i = 0; i < size-1; i++)
		{
			if (i == cursor)
			{
				junk = "RD /S /Q " + line[cursor];
				system(junk.c_str());
				std::string* tmp = new std::string[size-1];
				for (int j = 0, k = 0; j < size; j++)
				{
					if (j != cursor)
					{
						tmp[k] = options[j];
						k++;
					}
				}
				delete [] options;
				options = tmp;
			}
			else
				ofile << line[i] << std::endl;
		}
		ofile.close();
		size--;
		isUpdated = true;
		SetFileAttributes(word.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}
}

std::string startMenu::checkfor(std::string file) const
{
	std::ifstream ifile(file.c_str());
	if (!ifile)
	{
		std::ofstream ofile(file.c_str());
		ofile << "0";
		ofile.close();
		ifile.close();
		SetFileAttributes(file.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}
	return file;
}

