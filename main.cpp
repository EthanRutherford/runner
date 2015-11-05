#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <deque>
#include "menu.h"
#include "funcs.h"

MenuManager mManager;

tag cproj;
String project;
Project proj;
thread* tp = NULL,* sp = NULL;
std::deque<String> queue;
String running;
std::deque<String> names;
int counter = 0;

void Callback(String id);
void UpdateStatus()
{
	if (mManager.exists("menu2"))
	{
		int h = 20;
		for (int i = 0; i < proj.status.size(); i++)
		{
			String name = "flag" + proj.file[i];
			if (mManager.exists("menu2:" + name))
				mManager.deleteElement("menu2:" + name);
			String s;
			s = proj.status[i];
			mManager.newText(s, name, "menu2", 250, h);
			h += 40;
		}
		if (proj.status.size() > 1)
		{
			if (mManager.exists("menu2:flaglink"))
				mManager.deleteElement("menu2:flaglink");
			if (needsRelink(cproj))
				mManager.newText("*", "flaglink", "menu2", 250, h);
		}
	}
}
void QueueRebuild()
{
	if (proj.file.size() == 1)
	{
		if (proj.status[0] != ' ')
			Callback("menu2:comp");
		return;
	}
	bool needs = false;
	for (int i = 0; i < proj.file.size(); i++)
	{
		if (proj.status[i] != ' ')
		{
			needs = true;
			Callback("menu2:" + proj.file[i]);
		}
	}
	if (needs or needsRelink(cproj))
		Callback("menu2:link");
}
void UpdateQueueMenu()
{
	if (mManager.exists("qmenu"))
		mManager.deleteElement("qmenu");
	if (mManager.exists("menu2"))
	{
		mManager.newMenu("job queue", "qmenu", 620, 40, 160, 520);
		String words = "";
		if (tp)
		{
			words += running;
			counter++;
			if (counter > 20)
				words += '.';
			if (counter > 40)
				words += '.';
			if (counter > 60)
				words += '.';
			if (counter > 80)
				counter = 0;
			words += '\n';
		}
		for (int i = 0; i < names.size(); i++)
			words += names[i] + '\n';
		if (words != "")
			mManager.newText(words, "list", "qmenu", 20, 10);
	}
}
void SetMenu0(String words)
{
	if (words != "")
	{
		if (running == "compile")
			SetFailed(proj, "main");
		else
			SetFailed(proj, running);
		mManager.deleteElement("menu1");
		mManager.deleteElement("menu2");
		mManager.newMenu("result", "menu0", 20, 40, 760, 520);
		mManager.newButton("OK", "ok", "menu0", 20, 20);
		mManager.newText(words, "list", "menu0", 20, 60);
		queue.clear();	//temporary
		names.clear();	//temporary
	}
	else
	{
		if (queue.size() > 0)
		{
			Callback(queue.front());
			queue.pop_front();
			names.pop_front();
		}
	}
}
void SetMenu1()
{
	mManager.newMenu("Projects", "menu1", 20, 40, 280, 520);
	Parser rdr = loadFile();
	int x = 0;
	for (int i = 0; i < rdr.tags.size(); i++)
	{
		if (rdr.tags[i].name == "project")
		{
			for (int j = 0; j < rdr.tags[i].child.size(); j++)
			{
				if (rdr.tags[i].child[j].name == "name")
					mManager.newButton(rdr.tags[i].child[j].content, 
						rdr.tags[i].child[j].content, "menu1", 20, 20 + 40*x);
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
		"'o' key - open the project's files\n"
		"'d' key - invoke the gdb debugger\n"
		"'f' key - compile entire project using level 3 optimizations\n"
		"'a' key - compile and link all modified files\n"
		"'r' key - run the project\n"
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
	mManager.newMenu(title, "menu6", 320, 40, 460, 520);
	mManager.newText("Add new files", "nfo", "menu6", 20, 20);
	mManager.newTextBox("", "text", "menu6", 20, 60);
	mManager.newButton(".h File", "hfile", "menu6", 140, 100);
	mManager.newButton(".h & .cpp", "cppfile", "menu6", 20, 100);
	mManager.newText("Delete Files", "nfo2", "menu6", 20, 140);
	int h = 180;
	for (int j = 1; j < cproj.child.size(); j++)
	{
		if (cproj.child[j].name == "opt" or cproj.child[j].name == "main")
			continue;
		mManager.newButton("delete " + cproj.child[j].content,
			cproj.child[j].content, "menu6", 20, h);
		h += 40;
	}
	mManager.newButton("Done", "done", "menu6", 20, h);
}
void Callback(String id)
{
	if (id == "welcome:continue")
	{
		if (mManager.exists("welcome:dontshow") and 
			mManager.requestCheck("welcome:dontshow"))
			dontshow();
		mManager.deleteElement("welcome");
		SetMenu1();
	}
	else if (id == "menubar:file:new")
	{
		if (!mManager.exists("menu1"))
			return;
		project = "";
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
		mManager.newMenu("New Project", "menu3", 320, 40, 460, 520);
		mManager.newTextBox("", "text", "menu3", 20, 20);
		mManager.newButton("Set Name", "name", "menu3", 20, 60);
		mManager.linkTextBox("menu3:text", "menu3:name");
	}
	else if (id == "menubar:file:open")
	{
		if (project == "")
			return;
		openProject(cproj);
	}
	else if (id == "menubar:file:delete")
	{
		if (project == "" or !mManager.exists("menu1"))
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
		mManager.newMenu("Confirm", "menu4", 150, 40, 520, 120);
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
		if (project == "")
			return;
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu5"))
			mManager.deleteElement("menu5");
		SetEdit();
	}
	else if (id == "menubar:edit:copt")
	{
		if (project == "")
			return;
		String optstr;
		bool a = false, b = false, c = false, d = false, e = false, f = false;
		if (cproj.child[1].name == "opt")
		{
			optstr = cproj.child[1].content;
			if (optstr.find("-std=c++11") != std::string::npos)
				a = true;
			if (optstr.find("-U__STRICT_ANSI__") != std::string::npos)
				b = true;
			if (optstr.find("-lopengl32 -lglu32") != std::string::npos)
				c = true;
			if (optstr.find("-lfreeglut") != std::string::npos)
				d = true;
			if (optstr.find("-lglew32") != std::string::npos)
				e = true;
			if (optstr.find("-Wl,--subsystem,windows") != std::string::npos)
				f = true;
		}
		if (mManager.exists("menu2"))
			mManager.deleteElement("menu2");
		if (mManager.exists("menu6"))
			mManager.deleteElement("menu6");
		mManager.newMenu("Compiler Options", "menu5", 320, 40, 460, 520);
		mManager.newCheckBox("use std=c++11", "c++", "menu5", a, 20, 20);
		mManager.newCheckBox("use strict ansi", "ansi", "menu5", b, 20, 60);
		mManager.newCheckBox("use opengl", "gl", "menu5", c, 20, 100);
		mManager.newCheckBox("use glut", "glut", "menu5", d, 20, 140);
		mManager.newCheckBox("use glew", "glew", "menu5", e, 20, 180);
		mManager.newCheckBox("no console", "nocmd", "menu5", f, 20, 220);
		mManager.newButton("Apply", "apply1", "menu5", 20, 260);
		mManager.newTextBox(optstr, "advbox", "menu5", 20, 300, 420);
		mManager.newButton("Apply", "apply2", "menu5", 20, 340);
		mManager.linkTextBox("menu5:advbox", "menu5:apply2");
		mManager.newButton("Done", "done", "menu5", 140, 340);
	}
	else if (id == "menubar:edit:debug")
	{
		if (!mManager.exists("menu2"))
			return;
		if (!tp)
		{
			tp = new thread(debug, cproj);
			running = "debug";
		}
		else
		{
			queue.push_back(id);
			names.push_back("debug");
		}
	}
	else if (id == "menubar:edit:optimize")
	{
		if (!mManager.exists("menu2"))
			return;
		if (!tp)
		{
			tp = new thread(optimize, cproj);
			running = "optimize";
		}
		else
		{
			queue.push_back(id);
			names.push_back("optimize");
		}
	}
	else if (id == "menubar:edit:rebuild")
	{
		if (!mManager.exists("menu2"))
			return;
		QueueRebuild();
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
		Callback("menu1:" + cproj.child[0].content);
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
		bool upd = false;
		if (project != id.substr(6))
		{
			upd = true;
			project = id.substr(6);
			queue.clear();
			names.clear();
			proj.clear();
			sp->rejoin();
			Parser rdr = loadFile();
			int i = 0;
			for (; rdr.tags[i].child[0].content != project; i++);
			cproj = rdr.tags[i];
		}
		mManager.newMenu(project, "menu2", 320, 40, 280, 520);
		int h = 20;
		for (int j = 1; j < cproj.child.size(); j++)
		{
			if (cproj.child[j].name == "h")
				continue;
			if (cproj.child[j].name == "opt")
				continue;
			if (cproj.child[j].name == "main")
				mManager.newButton("compile " + cproj.child[j].content,
					"main", "menu2", 20, h);
			else
				mManager.newButton("compile " + cproj.child[j].content,
					cproj.child[j].content, "menu2", 20, h);
			if (upd)
				proj.emplace_back(cproj.child[j].content);
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
	}
	else if (id == "menu2:link")
	{
		if (!tp)
		{
			tp = new thread(_link, cproj);
			running = "link";
		}
		else
		{
			queue.push_back(id);
			names.push_back("link");
		}
	}
	else if (id == "menu2:run")
	{
		if (project == "")
			return;
		if (!tp)
		{
			tp = new thread(run, project);
			running = "run";
		}
		else
		{
			queue.push_back(id);
			names.push_back("run");
		}
	}
	else if (id == "menu2:comp")
	{
		if (!tp)
		{
			tp = new thread(compileSingle, cproj);
			running = "compile";
		}
		else
		{
			queue.push_back(id);
			names.push_back("compile");
		}
	}
	else if (id.compare(0, 5, "menu2") == 0)
	{
		if (!tp)
		{
			tp = new thread(compile, cproj.child[0].content, id.substr(6));
			running =  id.substr(6);
		}
		else
		{
			queue.push_back(id);
			names.push_back(id.substr(6));
		}
	}
	else if (id == "menu3:name")
	{
		String nproject = mManager.requestText("menu3:text");
		if (nproject == "")
			return;		//send a message?
		mManager.deleteElement("menu3");
		tag nproj;
		nproj.shorthand = false;
		nproj.name = "project";
		nproj.child.emplace_back();
		nproj.child.back().name = "name";
		nproj.child.back().content = nproject;
		nproj.child.back().shorthand = false;
		nproj.child.emplace_back();
		nproj.child.back().name = "main";
		nproj.child.back().content = "main";
		nproj.child.back().shorthand = false;
		addProject(nproj);
		mManager.deleteElement("menu1");
		SetMenu1();
		Callback("menu1:" + nproject);
		Callback("menubar:edit:edit");
	}
	else if (id.compare(0, 5, "menu4") == 0)
	{
		if (id.substr(6) == "yes")
		{
			Delete(project);
			project = "";
		}
		mManager.deleteElement("menu4");
		SetMenu1();
	}
	else if (id == "menu5:apply1")
	{
		String optstr = "";
		if (mManager.requestCheck("menu5:nocmd"))
			optstr += " -Wl,--subsystem,windows";
		if (mManager.requestCheck("menu5:glew"))
			optstr += " -lglew32";
		if (mManager.requestCheck("menu5:glut"))
			optstr += " -lfreeglut";
		if (mManager.requestCheck("menu5:gl"))
			optstr += " -lopengl32 -lglu32";
		if (mManager.requestCheck("menu5:c++"))
			optstr += " -std=c++11";
		if (mManager.requestCheck("menu5:ansi"))
			optstr += " -U__STRICT_ANSI__";
		editOptions(project, optstr, cproj);
		mManager.deleteElement("menu5:advbox");
		mManager.newTextBox(optstr, "advbox", "menu5", 20, 300, 420);
	}
	else if (id == "menu5:apply2")
	{
		editOptions(project, mManager.requestText("menu5:advbox"), cproj);
	}
	else if (id == "menu5:done")
	{
		mManager.deleteElement("menu5");
		String call = "menu1:" + project;
		project = "";
		Callback(call);
	}
	else if (id.compare(0, 5, "menu6") == 0)
	{
		if (id.substr(6) == "hfile")
		{
			String name = mManager.requestText("menu6:text");
			if (name == "")
				return;
			addTo(project, "h", name, cproj);
		}
		else if (id.substr(6) == "cppfile")
		{
			String name = mManager.requestText("menu6:text");
			if (name == "")
				return;
			addTo(project, "cpp", name, cproj);
		}
		else if (id.substr(6) == "done")
		{
			Callback("menu1:" + project);
			return;
		}
		else
		{
			deleteFrom(project, id.substr(6), cproj);
		}
		mManager.deleteElement("menu6");
		String call = "menu1:" + project;
		project = "";
		Callback(call);
		Callback("menubar:edit:edit");
	}
}
void Reshape (int width, int height)
{
	glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,(GLfloat)width,0,(GLfloat)height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void Idle()
{
	if (tp and !tp->running() and mManager.exists("menu2"))
	{
		String words = tp->getResult<String>();
		delete tp;
		tp = NULL;
		counter = 0;
		SetMenu0(words);
	}
	UpdateQueueMenu();
	
	if (sp and !sp->running())
	{
		if (mManager.exists("menu2") and project == proj.name)
			UpdateStatus();
		delete sp;
		sp = NULL;
	}
	if (!sp)
	{
		proj.name = project;
		sp = new thread(CheckStatus, &proj);
	}
}
void Display()
{
	
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
	mManager.newButton("Compile with Optimizations", "optimize", "edit");
	mManager.newButton("Rebuild Modified Files", "rebuild", "edit");
	mManager.newButton("Welcome", "welcome", "view");
	mManager.assignShortcut("menubar:file:open", 'o');
	mManager.assignShortcut("menubar:edit:debug", 'd');
	mManager.assignShortcut("menubar:edit:optimize", 'f');
	mManager.assignShortcut("menubar:edit:rebuild", 'a');
	mManager.assignShortcut("menubar:view:welcome", 'v');
	mManager.assignShortcut("menu2:run", 'r', true);
	Parser rdr = loadFile();
	int i = 0;
	for (; i + 1 < rdr.tags.size() and rdr.tags[i].name != "settings"; i++);
	if (i + 1 < rdr.tags.size() and rdr.tags[i].child[0].content == "true")
		SetMenu1();
	else
		SetWelcome();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800,600);
	glutInitWindowPosition(100, 150);
	glutCreateWindow(".runner10");
	
	initMenu();
	
	glutMainLoop();
	if (tp)
		delete tp;
	if (sp)
		delete sp;
}

