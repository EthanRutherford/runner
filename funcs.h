#include "xml.h"

String _system(String command, bool wait = true)
{
	SECURITY_ATTRIBUTES secAttr; 
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	secAttr.bInheritHandle = true; 
	secAttr.lpSecurityDescriptor = NULL; 
	HANDLE writeInChild, readFromChild;
	CreatePipe(&readFromChild, &writeInChild, &secAttr, 0);
	SetHandleInformation(readFromChild, HANDLE_FLAG_INHERIT, 0);
	PROCESS_INFORMATION procInfo; 
	STARTUPINFO startupInfo;
	ZeroMemory(&procInfo,    sizeof(procInfo));
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb         = sizeof(startupInfo);
	startupInfo.hStdError  = writeInChild;
	startupInfo.hStdOutput = writeInChild;
	startupInfo.dwFlags    = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE;
	char* commandCopy = (char*)malloc(command.size() + 1);
	strcpy(commandCopy, command.c_str());
	CreateProcess(NULL, commandCopy, NULL, NULL,
		true, 0, NULL, NULL, &startupInfo, &procInfo);
	free(commandCopy);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	CloseHandle(writeInChild);
	if (!wait)
		return "";
	String ret;
	DWORD dwRead;
	char chBuf[128];
	bool bSuccess = false;
	int printed = 0;
	int lines = 1;
	while (true)
	{
		bSuccess = ReadFile(readFromChild, chBuf, 128, &dwRead, NULL);
		if( ! bSuccess || dwRead == 0 ) break;
		for (int i = 0; i < dwRead; i++)
		{
			if (chBuf[i] == '\n')
			{
				ret += '\n';
				printed = 0;
				lines++;
			}
			else if (chBuf[i] > 31 and chBuf[i] < 127)
			{
				ret += chBuf[i];
				printed++;
			}
			else 
			{
				ret += '\n';
				printed = 0;
				lines++;
			}
			if (lines > 19)
				return ret + "...";
			if (printed > 78)
			{
				for (int j = 0; j < printed; j++)
				{
					if (ret[ret.length()-j] == ' ')
					{
						ret[ret.length()-j] = '\n';
						break;
					}
					if (j + 1 == printed)
						ret += '\n';
				}
				printed = 0;
				lines++;
			}
			if (lines > 19)
				return ret + "...";
		}
	}
	return ret;
}

std::string setDef(std::string name)
{
	for (int i = 0; i < name.length(); i++)
	{
		name[i] = toupper(name[i]);
		if (name[i] == '.')
			name[i] = '_';
	}
	return name;
}

void checkfor(std::string file)
{
	std::ifstream ifile(file.c_str());
	if (!ifile)
	{
		std::ofstream ofile(file.c_str());
		ofile.close();
		ifile.close();
		SetFileAttributes(file.c_str(), FILE_ATTRIBUTE_HIDDEN);
	}
}

Parser loadFile()
{
	Parser rdr;
	checkfor("projects.cfg");
	rdr.read("projects.cfg");
	return rdr;
}

void addProject(tag* proj)
{
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_NORMAL);
	Writer wrtr;
	wrtr.tags = loadFile().tags;
	wrtr.tags.emplace_back(proj);
	wrtr.write("projects.cfg");
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_HIDDEN);
	std::string folder = proj->child[0]->content;
	_system("cmd /c mkdir " + folder);
	for (int i = 1; i < proj->child.size(); i++)
	{
		if (proj->child[i]->name == "main")
		{
			std::string filename = proj->child[i]->content + ".cpp";
			std::ofstream ofile((folder + "\\" + filename).c_str());
			ofile << "int main()\n{\n\t\n}";
			ofile.close();
		}
		else if (proj->child[i]->name == "cpp")
		{
			std::string filename1 = proj->child[i]->content + ".h";
			std::ofstream ofile1(folder + "\\" + filename1.c_str());
			std::string def = setDef(filename1);
			ofile1 << "#ifndef " << def << "\n#define " << def << "\n\n\n\n#endif";
			ofile1.close();
			std::string filename2 = proj->child[i]->content + ".cpp";
			std::ofstream ofile2((folder + "\\" + filename2).c_str());
			ofile2 << "#include \"" << filename1 << "\"\n";
			ofile2.close();
		}
		else if (proj->child[i]->name == "h")
		{
			std::string filename = proj->child[i]->content + ".h";
			std::ofstream ofile((folder + "\\" + filename).c_str());
			ofile << "";
			ofile.close();
		}
	}
}

void openProject(tag* proj)
{
	_system("cmd /c start notepad++");
	for (int i = 0; i < proj->child.size(); i++)
	{
		String filename	= proj->child[0]->content;
		if (proj->child[i]->name == "main")
			_system("cmd /c " + filename + "\\" + proj->child[i]->content + ".cpp");
		else if (proj->child[i]->name == "h")
			_system("cmd /c " + filename + "\\" + proj->child[i]->content + ".h");
		else if (proj->child[i]->name == "cpp")
		{
			_system("cmd /c " + filename + "\\" + proj->child[i]->content + ".cpp");
			_system("cmd /c " + filename + "\\" + proj->child[i]->content + ".h");
		}
	}
}

void editOptions(String name, String optstr)
{
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_NORMAL);
	Writer wrtr;
	wrtr.tags = loadFile().tags;
	int i = 0;
	for (; wrtr.tags[i]->child[0]->content != name; i++)
		if (i == wrtr.tags.size()) return;
	if (wrtr.tags[i]->child[1]->name != "opt")
	{
		tag* child = new tag;
		child->name = "opt";
		child->content = optstr;
		child->shorthand = false;
		wrtr.tags[i]->child.insert(wrtr.tags[i]->child.begin()+1, child);
		wrtr.tags[i]->content = "\n\t@c" + wrtr.tags[i]->content;
	}
	else
		wrtr.tags[i]->child[1]->content = optstr;
	wrtr.write("projects.cfg");
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_HIDDEN);
}

void run(String name)
{
	name += ".exe";
	_system("cmd /c start " + name, false);
}

void Delete(String name)
{
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_NORMAL);
	Writer wrtr;
	wrtr.tags = loadFile().tags;
	int i = 0;
	for (; wrtr.tags[i]->child[0]->content != name; i++);
	_system("cmd /c RD /S /Q " + wrtr.tags[i]->child[0]->content, false);
	wrtr.tags.erase(wrtr.tags.begin()+i);
	wrtr.write("projects.cfg");
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_HIDDEN);
}

void deleteFrom(String name, String file)
{
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_NORMAL);
	Writer wrtr;
	wrtr.tags = loadFile().tags;
	int i = 0, j = 1;
	for (; wrtr.tags[i]->child[0]->content != name; i++)
		if (i == wrtr.tags.size()) return;
	for (; wrtr.tags[i]->child[j]->content != file; j++)
		if (j == wrtr.tags[i]->child.size()) return;
	wrtr.tags[i]->child.erase(wrtr.tags[i]->child.begin()+j);
	wrtr.tags[i]->content = "";
	wrtr.write("projects.cfg");
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_HIDDEN);
}

void addTo(String name, String ftype, String fname)
{
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_NORMAL);
	Writer wrtr;
	wrtr.tags = loadFile().tags;
	int i = 0;
	for (; wrtr.tags[i]->child[0]->content != name; i++)
		if (i == wrtr.tags.size()) return;
	wrtr.tags[i]->content = "";
	wrtr.tags[i]->child.emplace_back(new tag);
	wrtr.tags[i]->child.back()->name = ftype;
	wrtr.tags[i]->child.back()->content = fname;
	wrtr.tags[i]->child.back()->shorthand = false;
	wrtr.write("projects.cfg");
	SetFileAttributes("projects.cfg", FILE_ATTRIBUTE_HIDDEN);
	if (ftype == "cpp")
	{
		std::string filename1 = fname + ".h";
		std::ofstream ofile1(name + "\\" + filename1.c_str());
		std::string def = setDef(filename1);
		ofile1 << "#ifndef " << def << "\n#define " << def << "\n\n\n\n#endif";
		ofile1.close();
		std::string filename2 = fname + ".cpp";
		std::ofstream ofile2((name + "\\" + filename2).c_str());
		ofile2 << "#include \"" << filename1 << "\"\n";
		ofile2.close();
	}
	else if (ftype == "h")
	{
		std::string filename = fname + ".h";
		std::ofstream ofile((name + "\\" + filename).c_str());
		ofile << "";
		ofile.close();
	}
}

String compile(String projname, String filename)
{
	String query = "g++ -c " + projname + "\\" + filename + ".cpp -o "
		+ projname + "\\" + filename + ".o -std=c++11 -U__STRICT_ANSI__";
	String result = _system(query);
	if (result == "")
		return filename + ".cpp compiled successfully.";
	else
		return result;
}

String link(tag* proj)
{
	String list;
	String name;
	String options;
	for (int i = 0; i < proj->child.size(); i++)
	{
		if (proj->child[i]->name == "name")
			name = proj->child[i]->content;
		if (proj->child[i]->name == "cpp" or proj->child[i]->name == "main")
			list += name + "\\" + proj->child[i]->content + ".o ";
		if (proj->child[i]->name == "opt")
			options += proj->child[i]->content;
	}
	String query = "g++ -o " + name + ".exe " + list + options;
	String result = _system(query);
	if (result == "")
		return name + ".exe linked successfully.";
	else
		return result;
}

String compileSingle(tag* proj)
{
	String main;
	String name;
	String options;
	for (int i = 0; i < proj->child.size(); i++)
	{
		if (proj->child[i]->name == "name")
			name = proj->child[i]->content;
		if (proj->child[i]->name == "main")
			main = proj->child[i]->content;
		if (proj->child[i]->name == "opt")
			options += proj->child[i]->content;
	}
	String query = "g++ -o " + name + ".exe " + name + "/" + main + ".cpp " + options;
	String result = _system(query);
	if (result == "")
		return name + ".exe compiled successfully.";
	else
		return result;
}

