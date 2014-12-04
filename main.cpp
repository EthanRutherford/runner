#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/freeglut.h>
#include "menu.h"
#include "funcs.h"

int _screenw;
int _screenh;
int lasttime = 0;

MenuManager mManager;

tag* cproj;
tag* nproj;
String project;

void Reshape (int width, int height)
{
	_screenw = width;
	_screenh = height;
	glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,(GLfloat)width,0,(GLfloat)height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void Idle()
{
	int curtime = glutGet(GLUT_ELAPSED_TIME);
	int diff = curtime - lasttime;
	lasttime = curtime;
	Sleep(1);
	//use diff for anim, remove sleep when cycles > 0 ms
}
void Display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, .2, 0);
}
void SetMenu1()
{
	mManager.newMenu("Projects", "menu1", 20, 40, 300, 500);
	Parser rdr = loadFile();
	int x = 0;
	for (int i = 0; i < rdr.tags.size(); i++)
	{
		if (rdr.tags[i]->name == "project")
		{
			for (int j = 0; j < rdr.tags[i]->child.size(); j++)
			{
				if (rdr.tags[i]->child[j]->name == "name")
					mManager.newButton(rdr.tags[i]->child[j]->content, 
						rdr.tags[i]->child[j]->content, "menu1", 20, 20 + 40*x);
			}
			x++;
		}
	}
}
void SetWelcome(bool firstime = true)
{
	mManager.newMenu("Welcome", "welcome", 20, 40, 760, 540);
	mManager.newText("Welcome to .runner version 9.\n"
		"This program will manage all your programming needs.\n"
		"Managing project folders, compiling, linking, compiler options, etc.\n\n"
		"Shortcuts:\n"
		"'o' key - open the projects files\n"
		"'d' key - invoke the gdb debugger\n"
		"'v' key - view this page again\n\n"
		"If at anay point you need to view this page again, you can do that from\n"
		"the view tab, or by using the 'v' key shortcut.",
		"text", "welcome", 20, 20);
	if (firstime)
		mManager.newCheckBox("Don't show this screen on startup", 
			"dontshow", "welcome", false, 220, 400);
	mManager.newButton("Continue", "continue", "welcome", 300, 420);
}
void SetEdit()
{
	String title = "Edit '";
	title += project + "'";
	mManager.newMenu(title, "menu6", 340, 40, 440, 500);
	mManager.newText("Add new files", "nfo", "menu6", 20, 20);
	mManager.newTextBox("", "text", "menu6", 20, 60);
	mManager.newButton(".h File", "hfile", "menu6", 20, 100);
	mManager.newButton(".h & .cpp", "cppfile", "menu6", 140, 100);
	mManager.newText("Delete Files", "nfo2", "menu6", 20, 140);
	int h = 180;
	for (int j = 1; j < cproj->child.size(); j++)
	{
		if (cproj->child[j]->name == "opt" or cproj->child[j]->name == "main")
			continue;
		mManager.newButton("delete " + cproj->child[j]->content,
			cproj->child[j]->content, "menu6", 20, h);
		h += 40;
	}
	mManager.newButton("Done", "done", "menu6", 20, h);
}
void Callback(String id)
{
	if (id == "welcome:continue")
	{
		if (mManager.exists("welcome:dontshow") and mManager.requestCheck("welcome:dontshow"))
			dontshow();
		mManager.deleteElement("welcome");
		SetMenu1();
	}
	else if (id == "menubar:file:new")
	{
		if (!mManager.exists("menu1"))
			return;
		if (nproj != NULL)
			delete nproj;
		if (cproj != NULL)
		{
			delete cproj;
			cproj = NULL;
		}
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu3"))
			mManager.deleteElement("menu3");
		if (mManager.exists("menu4"))
			mManager.deleteElement("menu4");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		mManager.newMenu("New Project", "menu3", 340, 40, 440, 500);
		mManager.newTextBox("", "text", "menu3", 20, 20);
		mManager.newButton("Set Name", "name", "menu3", 20, 60);
		mManager.linkTextBox("menu3:text", "menu3:name");
		nproj = new tag;
		nproj->name = "project";
		nproj->shorthand = false;
	}
	else if (id == "menubar:file:open")
	{
		if (cproj == NULL)
			return;
		openProject(cproj);
	}
	else if (id == "menubar:file:delete")
	{
		if (cproj == NULL or !mManager.exists("menu1"))
			return;
		if (mManager.exists("menu1"))
			mManager.deleteElement("menu1");
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu3"))
			mManager.deleteElement("menu3");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		mManager.newMenu("Confirm", "menu4", 150, 40, 500, 120);
		mManager.newText("Are you sure you want to delete " + project + "?",
			"text", "menu4", 20, 20);
		mManager.newButton("Yes", "yes", "menu4", 20, 60);
		mManager.newButton("No", "no", "menu4", 150, 60);
	}
	else if (id == "menubar:file:exit")
	{
		exit(0);
	}
	else if (id == "menubar:edit:edit")
	{
		if (cproj == NULL)
			return;
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		SetEdit();
	}
	else if (id == "menubar:edit:copt")
	{
		if (cproj == NULL)
			return;
		String optstr;
		bool a = false, b = false, c = false, d = false, e = false;
		if (cproj->child[1]->name == "opt")
		{
			optstr = cproj->child[1]->content;
			if (optstr.find("-std=c++11") != std::string::npos)
				a = true;
			if (optstr.find("-U__STRICT_ANSI__") != std::string::npos)
				b = true;
			if (optstr.find("-lopengl32 -lglu32") != std::string::npos)
				c = true;
			if (optstr.find("-lfreeglut") != std::string::npos)
				d = true;
			if (optstr.find("-Wl,--subsystem,windows") != std::string::npos)
				e = true;
		}
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		mManager.newMenu("Compiler Options", "menu5", 340, 40, 300, 500);
		mManager.newCheckBox("use std=c++11", "c++", "menu5", a, 20, 20);
		mManager.newCheckBox("use strict ansi", "ansi", "menu5", b, 20, 60);
		mManager.newCheckBox("use opengl", "gl", "menu5", c, 20, 100);
		mManager.newCheckBox("use glut", "glut", "menu5", d, 20, 140);
		mManager.newCheckBox("no console", "nocmd", "menu5", e, 20, 180);
		mManager.newButton("Apply", "apply1", "menu5", 20, 220);
		mManager.newTextBox(optstr, "advbox", "menu5", 20, 260);
		mManager.newButton("Apply", "apply2", "menu5", 20, 300);
		mManager.linkTextBox("menu5:advbox", "menu5:apply2");
		mManager.newButton("Done", "done", "menu5", 140, 300);
	}
	else if (id == "menubar:edit:debug")
	{
		if (!mManager.exists("menu2"))
			return;
		String words = debug(cproj);
		if (words != "")
		{
			mManager.deleteElement("menu1");
			mManager.deleteElement("menu2");
			mManager.newMenu("result", "menu0", 20, 40, 760, 500);
			mManager.newButton("OK", "ok", "menu0", 20, 20);
			mManager.newText(words, "list", "menu0", 20, 60);
		}
	}
	else if (id == "menubar:view:welcome")
	{
		if (mManager.exists("menu0"))
			mManager.deleteElement("menu0");
		if (mManager.exists("menu1"))
			mManager.deleteElement("menu1");
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu3"))
			mManager.deleteElement("menu3");
		if (mManager.exists("menu4"))
			mManager.deleteElement("menu4");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		SetWelcome(false);
	}
	else if (id == "menu0:ok")
	{
		mManager.deleteElement("menu0");
		SetMenu1();
		Callback("menu1:" + cproj->child[0]->content);
	}
	else if (id.compare(0, 5, "menu1") == 0)
	{
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu3"))
			mManager.deleteElement("menu3");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		project = id.substr(6);
		mManager.newMenu(project, "menu2", 340, 40, 300, 500);
		Parser rdr = loadFile();
		int i = 0, j = 1, h = 20;
		for (; rdr.tags[i]->child[0]->content != project; i++);
		for (; j < rdr.tags[i]->child.size(); j++)
		{
			if (rdr.tags[i]->child[j]->name == "h")
				continue;
			if (rdr.tags[i]->child[j]->name == "opt")
				continue;
			mManager.newButton("compile " + rdr.tags[i]->child[j]->content,
				rdr.tags[i]->child[j]->content, "menu2", 20, h);
			h += 40;
		}
		if (h == 60)
		{
			h -= 40;
			mManager.deleteElement("menu2:main");
			mManager.newButton("compile", "comp", "menu2", 20, h);
		}
		else
			mManager.newButton("Link Executable", "link", "menu2", 20, h);
		mManager.newButton("Run Program", "run", "menu2", 20, h+40);
		cproj = rdr.tags[i];
	}
	else if (id == "menu2:link")
	{
		String words = link(cproj);
		mManager.deleteElement("menu1");
		mManager.deleteElement("menu2");
		mManager.newMenu("result", "menu0", 20, 40, 760, 500);
		mManager.newButton("OK", "ok", "menu0", 20, 20);
		mManager.newText(words, "list", "menu0", 20, 60);
	}
	else if (id == "menu2:run")
	{
		run(project);
	}
	else if (id == "menu2:comp")
	{
		String words = compileSingle(cproj);
		mManager.deleteElement("menu1");
		mManager.deleteElement("menu2");
		mManager.newMenu("result", "menu0", 20, 40, 760, 500);
		mManager.newButton("OK", "ok", "menu0", 20, 20);
		mManager.newText(words, "list", "menu0", 20, 60);
	}
	else if (id.compare(0, 5, "menu2") == 0)
	{
		String words = compile(cproj->child[0]->content, id.substr(6));
		mManager.deleteElement("menu1");
		mManager.deleteElement("menu2");
		mManager.newMenu("result", "menu0", 20, 40, 760, 500);
		mManager.newButton("OK", "ok", "menu0", 20, 20);
		mManager.newText(words, "list", "menu0", 20, 60);
	}
	else if (id == "menu3:name")
	{
		project = mManager.requestText("menu3:text");
		if (project == "")
			return;		//send a message?
		mManager.deleteElement("menu3");
		nproj->child.emplace_back(new tag);
		nproj->child.back()->name = "name";
		nproj->child.back()->content = project;
		nproj->child.back()->shorthand = false;
		nproj->child.emplace_back(new tag);
		nproj->child.back()->name = "main";
		nproj->child.back()->content = "main";
		nproj->child.back()->shorthand = false;
		addProject(nproj);
		delete nproj;
		nproj = NULL;
		mManager.deleteElement("menu1");
		SetMenu1();
		Callback("menu1:" + project);
		Callback("menubar:edit:edit");
	}
	else if (id.compare(0, 5, "menu4") == 0)
	{
		if (id.substr(6) == "yes")
		{
			Delete(project);
			delete cproj;
			project = "";
			cproj = NULL;
		}
		mManager.deleteElement("menu4");
		SetMenu1();
	}
	else if (id == "menu5:apply1")
	{
		String optstr = "";
		if (mManager.requestCheck("menu5:nocmd"))
			optstr += " -Wl,--subsystem,windows";
		if (mManager.requestCheck("menu5:glut"))
			optstr += " -lfreeglut";
		if (mManager.requestCheck("menu5:gl"))
			optstr += " -lopengl32 -lglu32";
		if (mManager.requestCheck("menu5:c++"))
			optstr += " -std=c++11";
		if (mManager.requestCheck("menu5:ansi"))
			optstr += " -U__STRICT_ANSI__";
		editOptions(project, optstr);
		mManager.deleteElement("menu5:advbox");
		mManager.newTextBox(optstr, "advbox", "menu5", 20, 260);
	}
	else if (id == "menu5:apply2")
	{
		editOptions(project, mManager.requestText("menu5:advbox"));
	}
	else if (id == "menu5:done")
	{
		mManager.deleteElement("menu5");
		Callback("menu1:" + project);
	}
	else if (id.compare(0, 5, "menu6") == 0)
	{
		if (id.substr(6) == "hfile")
		{
			String name = mManager.requestText("menu6:text");
			if (name == "")
				return;
			addTo(project, "h", name);
		}
		else if (id.substr(6) == "cppfile")
		{
			String name = mManager.requestText("menu6:text");
			if (name == "")
				return;
			addTo(project, "cpp", name);
		}
		else if (id.substr(6) == "done")
		{
			Callback("menu1:" + project);
			return;
		}
		else
		{
			deleteFrom(project, id.substr(6));
		}
		mManager.deleteElement("menu6");
		Callback("menu1:" + project);
		Callback("menubar:edit:edit");
	}
}
void initMenu()
{
	mManager.init();
	mManager.DisplayFunc(Display);
	mManager.CallbackFunc(Callback);
	glutIdleFunc(Idle);
	mManager.ReshapeFunc(Reshape);
	mManager.newTab("File", "file");
	mManager.newTab("Edit", "edit");
	mManager.newTab("View", "view");
	mManager.newButton("New", "new", "file");
	mManager.newButton("Open Project Files", "open", "file");
	mManager.newButton("Delete", "delete", "file");
	mManager.newButton("Exit", "exit", "file");
	mManager.newButton("Edit Project", "edit", "edit");
	mManager.newButton("Compiler Options", "copt", "edit");
	mManager.newButton("Debugger", "debug", "edit");
	mManager.newButton("Welcome", "welcome", "view");
	mManager.assignShortcut("menubar:file:open", 'o');
	mManager.assignShortcut("menubar:edit:debug", 'd');
	Parser rdr = loadFile();
	int i = 0;
	for (; i + 1 < rdr.tags.size() and rdr.tags[i]->name != "settings"; i++);
	if (rdr.tags[i]->name == "settings" and rdr.tags[i]->child[0]->content == "true")
		SetMenu1();
	else
		SetWelcome();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(800,600);
	glutInitWindowPosition(100, 150);
	glutCreateWindow(".runner9");
	
	initMenu();
	
	glutMainLoop();
}

