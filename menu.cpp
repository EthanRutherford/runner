#include "menu.h"
#include <fstream>
#include <algorithm>
#include <time.h>
#include <iostream>

//convenience functions
String date = "";						//date grabbed
String getTimeDate(){					//get time string
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	String res = asctime(timeinfo);
	res.pop_back();
	return res;
}
bool logError(String message){			//error logging helper
	String name = "errorlog.txt";
	std::ofstream debug;
	debug.open(name.c_str(), std::ofstream::app);
	if (date == "")
	{
		date = getTimeDate();
		debug << date << "\n";
	}
	debug << message << "\n\n";
	debug.close();
	return false;
}
bool contained(area a, area b){			//tests if area b is contained in area a
	if (a.x > b.x or a.x+a.w < b.x+b.w) return false;
	if (a.y < b.y or a.y-a.h > b.y-b.h) return false;
	return true;
}
bool overlap(area a, area b){			//tests if two areas overlap
	if(a.x+a.w <= b.x or a.x >= b.x+b.w) return false;
	if(a.y-a.h >= b.y or a.y <= b.y-b.h) return false;
	return true;
}
bool overlap(area a, int x, int y){		//test if the point overlaps the button
	if(x < a.x or x > a.x+a.w) return false;
	if(y > a.y or y < a.y-a.h) return false;
	return true;
}
bool overlap(textbox* t, int x, int y){	//test overlap, set cursor
	if (overlap(t->a, x, y))
	{
		t->cursor = (x-t->a.x-2)/9 + t->offset;
		if (t->cursor > t->contents.length())
			t->cursor = t->contents.length();
		return true;
	}
	return false;
}
void drawarea(area a){					//draw a rectangle
	glVertex2d(a.x,		a.y		);
	glVertex2d(a.x+a.w,	a.y		);
	glVertex2d(a.x+a.w, a.y-a.h	);
	glVertex2d(a.x,		a.y-a.h	);
}

//elements
void button::draw(int yoff)
{
	area a2 = a;
	a2.y += yoff;
	glBegin(GL_QUADS);
	if (clicked)	glColor3d(0.1, 0.1, 0.1);
	else			glColor3d(0.2, 0.2, 0.2);
	drawarea(a2);
	glEnd();
	glBegin(GL_LINE_LOOP);
	if (clicked)	glColor3d(0,0,0);
	else			glColor3d(0.1, 0.1, 0.1);
	drawarea(a2);
	glEnd();
	if (clicked)	glColor3d(.8,.8,.8);
	else			glColor3d(1, 1, 1);
	drawText(name, a2.x + 2, a2.y - (a2.h/2) - 5);
}
tab::~tab()
{
	for (int i = 0; i < buttons.size(); i++)
		delete buttons[i];
}
void tab::draw(int yoff)
{
	glColor3d(.6, .6, .8);
	if (clicked or open)
	{
		if (clicked)	glColor3d(0.1, 0.1, 0.1);
		else			glColor3d(0.2, 0.2, 0.2);
		glBegin(GL_QUADS);
		drawarea(a);
		glEnd();
		if (clicked)	glColor3d(.8,.8,.8);
		else			glColor3d(1, 1, 1);
	}
	drawText(name, a.x+2, a.y - (a.h/2) - 5);
	if (!open) return;
	for (int i = 0; i < buttons.size(); i++)
		buttons[i]->draw();
}
void textbox::draw(int yoff)
{
	area a2 = a;
	a2.y += yoff;
	glBegin(GL_QUADS);
	if (clicked)	glColor3d(0, 0, 0);
	else			glColor3d(0.1, 0.1, 0.1);
	drawarea(a2);
	glEnd();
	glBegin(GL_LINE_LOOP);
	if (clicked)	glColor3d(.4,.5,.6);
	else			glColor3d(0.3, 0.4, 0.5);
	drawarea(a2);
	glEnd();
	//draw cursor
	if (clicked)
	{
		glBegin(GL_LINE_STRIP);
		int x = a2.x + 2 + ((cursor-offset) * 9);
		glVertex2d(x,	a2.y-(a2.h/2-6));
		glVertex2d(x,	a2.y-(a2.h/2+6));
		glEnd();
	}
	if (clicked)	glColor3d(1,1,1);
	else			glColor3d(.8, .8, .8);
	//determine portion of contents to display
	int maxlength = (a2.w - 4) / 9;
	if (cursor-offset > maxlength)
		offset++;
	if (cursor-offset < 0)
		offset = std::max(0, offset - 5);
	int length = maxlength;
	drawText(contents.substr(offset, length), a2.x + 2, a2.y - (a2.h/2) - 5);
}
String textbox::log(char key)
{
	if (key == (char)8)
	{
		if (cursor > 0)
			contents.erase(--cursor, 1);
	}
	else if (key == (char)127)
	{
		if (cursor < contents.length())
			contents.erase(cursor, 1);
	}
	else if (key == (char)13)
	{
		clicked = false;
		return pseudoId;
	}
	else if ((int)key > 30)
		contents.insert(cursor++, 1, key);
	return "";
}
void textbox::special(int key)
{
	if (key == GLUT_KEY_RIGHT and cursor < contents.length())
		cursor++;
	if (key == GLUT_KEY_LEFT and cursor > 0)
		cursor--;
	if (key == GLUT_KEY_UP)
		cursor = offset = 0;
}
void textarea::draw(int yoff)
{
	area a2 = a;
	a2.y += yoff;
	glColor3d(.8, .8, .8);
	for (int i = start; i < end; i++)
		drawText(lines[i], a2.x, a2.y-15-(i*20));
}
void textarea::prepare(int y, int h, int yoff)
{
	start = 0; 
	end = lines.size();
	int diff1 = (a.y + yoff) - y;
	if (diff1 > 0)
		start += (diff1+20)/20;
	int diff2 = (a.y + yoff - a.h) - (y - h);
	if (diff2 < 0)
		end += (diff2-20)/20;
}
void checkbox::draw(int yoff)
{
	area a2 = a;
	a2.y += yoff;
	if (clicked)	glColor3d(.5, .5, .7);
	else			glColor3d(.7, .7, .9);
	glBegin(GL_QUADS);
	drawarea(a2);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3d(0.0, 0.0, 0.1);
	drawarea(a2);
	glEnd();
	if (checked)
		drawText("x", a2.x, a2.y - (a2.h/2) - 4);
	glColor3d(.8, .8, .8);
	drawText(name, a2.x + 12, a2.y - (a2.h/2) - 5);
}
void scrollbar::draw(int yoff)
{
	glColor3d(.5, .5, .7);
	glBegin(GL_QUADS);
	drawarea(a);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3d(0.0, 0.0, 0.1);
	drawarea(a);
	glEnd();
	b.y = a.y - position;
	
	if (clicked)	glColor3d(0.2, 0.2, 0.2);
	else			glColor3d(0.3, 0.3, 0.3);
	glBegin(GL_QUADS);
	drawarea(b);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3d(0.0, 0.0, 0.1);
	drawarea(b);
	glEnd();
}

//Menu
Menu::~Menu()
{
	for (int i = 0; i < elems.size(); i++)
		delete elems[i];
	if (bar)
		delete bar;
}
void Menu::draw()
{
	glBegin(GL_QUADS);
	glColor3d(0.17, 0.2, 0.25);
	drawarea(a);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3d(0, 0, 0.1);
	drawarea(a);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2d(a.x,		a.y-20);
	glVertex2d(a.x+a.w,	a.y-20);
	glEnd();
	glColor3d(.6, .6, .8);
	drawText(name, a.x+2, a.y-15);
	int yoff = bar? bar->position / bar->ratio : 0;
	for (int i = 0; i < elems.size(); i++)
	{
		area a2 = elems[i]->a;
		a2.y += yoff;
		if (elems[i]->Type() == _textarea)
		{
			((textarea*)elems[i])->prepare(a.y-20, a.h-20, yoff);
			elems[i]->draw(yoff);
		}
		else if (contained(area(a.x, a.y-20, a.w, a.h-20), a2))
			elems[i]->draw(yoff);
	}
	if (bar)
		bar->draw();
}
String Menu::click(int x, int y)
{
	int yoff = bar? bar->position / bar->ratio : 0;
	//close active textbox (will reactivate if same textbox is clicked)
	if (active)
		active->clicked = false;
	active = NULL;
	//check for scrollbar
	if (bar and overlap(bar->b, x, y))
	{
		bar->clicked = true;
		return "";
	}
	//see if the click hit an elem, and perform action
	for (int i = 0; i < elems.size(); i++)
	{
		area a2 = elems[i]->a;
		a2.y += yoff;
		if (overlap(a2, x, y))
		{
			elems[i]->clicked = true;
			if (elems[i]->Type() == element::_button)
				return id + ":" + elems[i]->id;
			if (elems[i]->Type() == element::_textbox)
			{
				active = (textbox*)elems[i];
				overlap((textbox*)elems[i], x, y);
			}
		}
	}
	//return a miss
	return "";
}
void Menu::release()
{
	if (bar)
		bar->clicked = false;
	for (int i = 0; i < elems.size(); i++)
	{
		if (elems[i]->Type() == element::_checkbox and elems[i]->clicked)
		{
			checkbox* c = (checkbox*)elems[i];
			c->checked = !c->checked;
		}
		if (elems[i]->Type() != element::_textbox)
			elems[i]->clicked = false;
	}
}
bool Menu::newButton(String n, String id, int x, int y)
{
	y = a.y - y - 20;
	x = a.x + x;
	//ensure real id
	if (id == "")
		return logError("Menu button needs real id.");
	//auto-assign size
	int w = std::max((int)n.length()*9+14, 100);
	int h = 20;
	area a2 = a;
	a2.y -= 20;
	a2.h -= 20;
	if (bar)
		a2.h = bar->innerh;
	//adjust height if necessary
	int diff = (a2.y - a2.h) - (y - h - 20);
	if (diff > 0)
		a2.h += diff;
	//ensure button is inside menu
	if (!contained(a2, area(x, y, w, h)))
		return logError("Button: " + n + " must be inside menu.");
	//set scroll bar if height was adjusted
	if (a.h - 20 != a2.h)
		setScrollBar(a2.h);
	//ensure buttons don't overlap/ ids don't conflict
	for (int i = 0; i < elems.size(); i++)
		if (overlap(elems[i]->a, area(x, y, w, h)) or id == elems[i]->id)
			return logError("Id conflict between " + elems[i]->name + " and " + n + ".");
	
	//create and add the button
	button* b = new button;
	b->a.x = x;
	b->a.y = y;
	b->a.w = w;
	b->a.h = h;
	b->name = n;
	b->id = id;
	b->clicked = false;
	elems.emplace_back(b);
	return true;
}
bool Menu::newTextBox(String n, String id, int x, int y)
{
	y = a.y - y - 20;
	x = a.x + x;
	//ensure box id is valid
	if (id == "")
		return logError("Invalid textbox id.");
	area a2 = a;
	a2.y -= 20;
	a2.h -= 20;
	if (bar)
		a2.h = bar->innerh;
	//adjust height if necessary
	int diff = (a2.y - a2.h) - (y - 40);
	if (diff > 0)
		a2.h += diff;
	//ensure box is inside menu
	if (!contained(a2, area(x, y, 200, 20)))
		return logError("Textbox: " + n + " must be inside menu.");
	//set scroll bar if height was adjusted
	if (a.h - 20 != a2.h)
		setScrollBar(a2.h);
	//ensure box id does not conflict
	for (int i = 0; i < elems.size(); i++)
		if (id == elems[i]->id)
			return logError("Duplicate textbox id: " + id + ".");
		
	//create and add textbox
	textbox* t = new textbox;
	t->a.x = x;
	t->a.y = y;
	t->a.w = 200;
	t->a.h = 20;
	t->clicked = false;
	t->contents = n;
	t->cursor = 0;
	t->offset = 0;
	t->name = n;
	t->id = id;
	elems.emplace_back(t);
	return true;
}
bool Menu::newText(String text, String id, int x, int y)
{
	y = a.y - y - 20;
	x = a.x + x;
	//ensure there is real text
	if (text == "")
		return logError("Textarea must have some text.");
	//can't check size early due to unknown height/width
	textarea* T = new textarea;
	T->a.x = x;
	T->a.y = y;
	T->a.w = 0;
	T->a.h = 0;
	T->id = id;
	int num = 0;
	while (num < text.length())
	{
		String line = "";
		int width = 4;
		while (num < text.length() and text[num] != '\n')
		{
			line += text[num++];
			width += 9;
		}
		T->a.w = std::max(width, T->a.w);
		T->a.h += 20;
		T->lines.emplace_back(line);
		num++;
	}
	area a2 = a;
	a2.y -= 20;
	a2.h -= 20;
	if (bar)
		a2.h = bar->innerh;
	//adjust height if necessary
	int diff = (a2.y - a2.h) - (T->a.y - T->a.h - 20);
	if (diff > 0)
		a2.h += diff;
	//make sure text is contained
	if (!contained(a2, T->a))
	{
		delete T;
		return logError("TextArea with contents:\n'" + text + "'\nwas not contained n menu.");
	}
	//set scroll bar if height was adjusted
	if (a.h - 20 != a2.h)
		setScrollBar(a2.h);
	elems.emplace_back(T);
	return true;
}
bool Menu::newCheckBox(String n, String id, bool check, int x, int y)
{
	y = a.y - y - 20;
	x = a.x + x;
	//ensure box id is valid
	if (id == "")
		return logError("Invalid checkbox id.");
	area a2 = a;
	a2.y -= 10;
	a2.h -= 10;
	if (bar)
		a2.h = bar->innerh;
	//adjust height if necessary
	int diff = (a2.y - a2.h) - (y - 40);
	if (diff > 0)
		a2.h += diff;
	//ensure box is inside menu
	if (!contained(a2, area(x, y, 200, 20)))
		return logError("Textbox: " + n + " must be inside menu.");
	//set scroll bar if height was adjusted
	if (a.h - 10 != a2.h)
		setScrollBar(a2.h);
	//ensure box id does not conflict
	for (int i = 0; i < elems.size(); i++)
		if (id == elems[i]->id)
			return logError("Duplicate checkbox id: " + id + ".");
	checkbox* c = new checkbox;
	c->a.x = x;
	c->a.y = y;
	c->a.h = 10;
	c->a.w = 10;
	c->checked = check;
	c->clicked = false;
	c->name = n;
	c->id = id;
	elems.emplace_back(c);
	return true;
}
bool Menu::setScrollBar(int innerh)
{
	if (!bar)
		bar = new scrollbar;
	bar->a.x = a.x + a.w - 30;
	bar->a.y = a.y - 20;
	bar->a.h = a.h - 20;
	bar->a.w = 20;
	bar->b.w = bar->a.w;
	bar->ratio = ((double)bar->a.h/(double)innerh);
	bar->b.h =  bar->ratio * bar->a.h;
	bar->b.x = bar->a.x;
	bar->b.y = bar->a.y;
	bar->clicked = false;
	bar->innerh = innerh;
	bar->position = 0;
	return true;
}
bool Menu::inside(int x, int y)
{
	return overlap (a, x, y);
}
void Menu::reshape(int width, int height)
{
	a.y -= height;
	for (int i = 0; i < elems.size(); i++)
		elems[i]->a.y -= height;
	if (bar)
		bar->a.y -= height;
}

//MenuBar
MenuBar::~MenuBar()
{
	for (int i = 0; i < tabs.size(); i++)
		delete tabs[i];
}
void MenuBar::draw()
{
	glBegin(GL_QUADS);
	glColor3d(0.17, 0.2, 0.25);
	drawarea(a);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glColor3d(0, 0, 0.1);
	drawarea(a);
	glEnd();
	for (int i = 0; i < tabs.size(); i++)
		tabs[i]->draw();
}
String MenuBar::click(int x, int y)
{
	bool tabbed = false;
	for (int i = 0; i < tabs.size(); i++)
	{
		//if the tab is open, check if a button was clicked
		if (tabs[i]->open)
			for (int j = 0; j < tabs[i]->buttons.size(); j++)
				if (overlap(tabs[i]->buttons[j]->a, x, y))
				{
					tabs[i]->buttons[j]->clicked = true;
					bttn = true;
					return id + ":" + tabs[i]->id + ":" + tabs[i]->buttons[j]->id;
				}
		//handle clicks on tabs
		if (overlap(tabs[i]->a, x, y) and !tabbed)
			tabbed = tabs[i]->clicked = true;
		else	//close all tabs not clicked
			tabs[i]->open = false;
	}
	//return a miss
	return "";
}
void MenuBar::release()
{
	for (int i = 0; i < tabs.size(); i++)
	{
		//clear clicked flag for all buttons
		for (int j = 0; j < tabs[i]->buttons.size(); j++)
			tabs[i]->buttons[j]->clicked = false;
		//if tab was clicked, open tab
		if (tabs[i]->clicked)
			tabs[i]->open = !tabs[i]->open;
		//clear clicked flag for all tabs
		tabs[i]->clicked = false;
		//if a button was clicked, close the tab
		if (bttn == true)
			tabs[i]->open = false;
	}
	//clear button flag
	bttn = false;
}
void MenuBar::closeTabs()
{
	for (int i = 0; i < tabs.size(); i++)
		tabs[i]->open = false;
}
bool MenuBar::newButton(String n, String id, String parent)
{
	//ensure real id
	if (id == "")
		return logError("Menubar button must have real id.");
	//ensure parent exists and find parent
	int tab = -1;
	for (int i = 0; i < tabs.size(); i++)
		if (tabs[i]->id == parent)
			tab = i;
	if (tab == -1)
		return logError("For button: " + id + ", parent tab: " + parent + " not found.");
	//ensure button ids don't conflict
	for (int i = 0; i < tabs[tab]->buttons.size(); i++)
		if (id == tabs[tab]->buttons[i]->id)
			return logError("Duplicate button id: " + id + ".");
	
	//if name is too big, make the buttons wider
	if (n.length()*9+14 > tabs[tab]->buttonsize)
	{
		//add a little extra room at the end
		tabs[tab]->buttonsize = n.length()*9+14;
		for (int i = 0; i < tabs[tab]->buttons.size(); i++)
			tabs[tab]->buttons[i]->a.w = tabs[tab]->buttonsize;
	}
	
	//create and add the button
	button* b = new button;
	b->a.x = tabs[tab]->a.x;
	b->a.y = tabs[tab]->buttons.size() > 0 
		? tabs[tab]->buttons.back()->a.y-20 : tabs[tab]->a.y-20;
	b->a.w = tabs[tab]->buttonsize;
	b->a.h = 20;
	b->name = n;
	b->id = id;
	b->clicked = false;
	tabs[tab]->buttons.emplace_back(b);
	return true;
}
bool MenuBar::newTab(String n, String id)
{
	//ensure real id
	if (id == "")
		return logError("Tab must have real id.");
	//ensure ids don't conflict
	for (int i = 0; i < tabs.size(); i++)
		if (id == tabs[i]->id)
			return logError("Duplicate tab id: " + id + ".");
		
	//create and add tab
	tab* t = new tab;
	t->a.x = nextX;
	t->a.y = a.y;
	t->a.w = n.length()*9+14;
	nextX += t->a.w;
	t->a.h = 20;
	t->buttonsize = 100;
	t->name = n;
	t->id = id;
	t->clicked = false;
	t->open = false;
	tabs.emplace_back(t);
	return true;
}
bool MenuBar::inside(int x, int y)
{
	for (int i = 0; i < tabs.size(); i++)
	{
		//if tab is open, check if inside tab area
		if (tabs[i]->open and tabs[i]->buttons.size() > 0)
		{
			//if below the bottom of the tab, its not inside
			area* A = &tabs[i]->buttons.back()->a;
			if (y < A->y - A->h)
				return false;
			//if above bottom of tab and within x bounds, is inside
			if (x > tabs[i]->a.x and x < tabs[i]->a.x+tabs[i]->buttonsize)
				return true;
		}
	}
	//if below menubar min, false; if above min, true
	if(y < a.y-a.h) return false;
	return true;
}
void MenuBar::reshape(int w, int h)
{
	//set position and width relative to window
	a.x = 0; a.y = h; a.w = w; a.h = 20;
	int curX = 0;
	for (int i = 0; i < tabs.size(); i++)
	{
		//reposition tabs
		tabs[i]->a.x = curX;
		tabs[i]->a.y = a.y;
		int curY = a.y-20;
		for (int j = 0; j < tabs[i]->buttons.size(); j++)
		{
			//reposition buttons in tabs
			tabs[i]->buttons[j]->a.x = curX;
			tabs[i]->buttons[j]->a.y = curY;
			curY -= 20;
		}
		curX += tabs[i]->a.w;
	}
	//ensure correct position for future tabs
	nextX = curX;
}

//MenuManager glut callbacks and MenuManager callthroughs
MenuManager* instance;
void (*cllbck)(String);
void (*dsply)();
void (*ms)(int, int, int, int);
void (*kybrd)(unsigned char, int, int);
void (*spcl)(int, int, int);
void (*rshp)(int, int);
void _Display(){
	if (dsply) (*dsply)();
	instance->draw();
	glFlush();
	glutPostRedisplay();
}
void _Mouse(int button, int state, int x, int y){
	String R = instance->mouse(button, state, x, y);
	if (cllbck and R != "") (*cllbck)(R);
	if (ms) (*ms)(button, state, x, y);
}
void _ActiveMouse(int x, int y){
	instance->point(x, y);
}
void _PassiveMouse(int x, int y){
	instance->point(x, y, false);
}
void _Keyboard(unsigned char key, int x, int y){
	String R = instance->keyPressed(key);
	if (cllbck and R != "") (*cllbck)(R);
	if (kybrd) (*kybrd)(key, x, y);
}
void _Special(int key, int x, int y){
	instance->keySpecial(key);
	if (spcl) (*spcl)(key, x, y);
}
void _Reshape(int width, int height){
	instance->reshape(width, height);
	if (rshp) (*rshp)(width, height);
}
void MenuManager::DisplayFunc(void (*func)()){
	dsply = func;
}
void MenuManager::MouseFunc(void (*func)(int, int, int, int)){
	ms = func;
}
void MenuManager::KeyboardFunc(void (*func)(unsigned char, int, int)){
	kybrd = func;
}
void MenuManager::SpecialFunc(void (*func)(int, int, int)){
	spcl = func;
}
void MenuManager::ReshapeFunc(void (*func)(int, int)){
	rshp = func;
}

//MenuManager functions
MenuManager::MenuManager() : topmenu("", "menubar"), height(-1)
{
	if (instance == NULL)
		instance = this;
	else
	{
		logError("Attempt to instantiate second instance of MenuManager.");
		exit(1);
	}
}
MenuManager::~MenuManager()
{
	for (int i = 0; i < menus.size(); i++)
		delete menus[i];
}
void MenuManager::init()
{
	glutDisplayFunc(_Display);
	glutMouseFunc(_Mouse);
	glutMotionFunc(_ActiveMouse);
	glutPassiveMotionFunc(_PassiveMouse);
	glutKeyboardFunc(_Keyboard);
	glutSpecialFunc(_Special);
	glutReshapeFunc(_Reshape);
}
void MenuManager::CallbackFunc(void (*func)(String))
{
	cllbck = func;
}
void MenuManager::draw()
{
	//clear the transformation so menus are in screen coords
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	for (int i = 0; i < menus.size(); i++)
		menus[i]->draw();
	//draw menubar last, to ensure it always being on top
	topmenu.draw();	
}
String MenuManager::mouse(int button, int state, int x, int y)
{
	y = height - y;
	if (button == 0 and state == 0)
	{
		//test menubar first, in case tab overlaps a menu
		if (topmenu.inside(x, y))
		{
			result = topmenu.click(x, y);
			return "";
		}
		else
			topmenu.closeTabs();
		//test all menus
		for (int i = 0; i < menus.size(); i++)
		{
			if (menus[i]->inside(x, y))
			{
				result = menus[i]->click(x, y);
				return "";
			}
		}
		//store a miss
		result = "";
	}
	if (button == 0 and state == 1)
	{
		//perform mb release logic on all menus
		for (int i = 0; i < menus.size(); i++)
			menus[i]->release();
		topmenu.release();
		//register the clicked event on release
		return result;
	}
	return "";
}
void MenuManager::point(int x, int y, bool active)
{
	if (active)
	{
		for (int i = 0; i < menus.size(); i++)
		{
			if (menus[i]->bar and menus[i]->bar->clicked)
			{
				menus[i]->bar->position += my - (height - y);
				if (menus[i]->bar->position < 0)
					menus[i]->bar->position = 0;
				if (menus[i]->bar->position + menus[i]->bar->b.h > menus[i]->bar->a.h)
					menus[i]->bar->position = menus[i]->bar->a.h - menus[i]->bar->b.h;
			}
		}
	}
	mx = x, my = height - y;
}
bool MenuManager::newMenu(String name, String id, int x, int y, int w, int h)
{
	if (height == -1)
		height = glutGet(GLUT_WINDOW_HEIGHT);
	y = height - y;
	//ensure real id
	if (id == "")
		return logError("Menu must have real id.");
	//ensure id does not conflict
	if (exists(id))
		return logError("Menu id conflict: " + id + ".");
	
	//create and add menu
	Menu* m = new Menu(name, id, x, y, w, h);
	menus.emplace_back(m);
	return true;
}
bool MenuManager::newButton(String n, String id, String parent, int x, int y)
{
	//get parent and add button
	element* p = getElem(parent);
	if (p == NULL)
		p = getElem("menubar:" + parent);
	if (p == NULL)
		return logError("When creating button '" + n + "': Parent '" + parent 
		+ "' does not exist");
	if (p->Type() == element::_tab)
		return topmenu.newButton(n, id, parent);
	else if (p->Type() == element::_Menu)
		return ((Menu*)p)->newButton(n, id, x, y);
	//if parent isn't menu or tab
	return logError("When creating button '" + n + "': Parent '" + parent 
		+ "' is not valid.");
}
bool MenuManager::newTextBox(String n, String id, String parent, int x, int y)
{
	//search the menu names until match, then add box
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->id == parent)
			return menus[i]->newTextBox(n, id, x, y);
	//if parent does not exist
	return logError("Parent \"" + parent + "\" does not exist.");
}
bool MenuManager::newCheckBox(String n, String id, String parent, bool check, int x, int y)
{
	//search the menu names until match, then add box
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->id == parent)
			return menus[i]->newCheckBox(n, id, check, x, y);
	//if parent does not exist
	return logError("Parent \"" + parent + "\" does not exist.");
}
bool MenuManager::newText(String text, String id, String parent, int x, int y)
{
	//search the menu names until match, then add text
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->id == parent)
			return menus[i]->newText(text, id, x, y);
	//if parent does not exist
	return logError("Parent \"" + parent + "\" does not exist.");
}
bool MenuManager::setScrollBar(String parent, int innerh)
{
	//search the menu names until match, then add bar
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->id == parent)
			return menus[i]->setScrollBar(innerh);
	//if parent does not exist
	return logError("Parent \"" + parent + "\" does not exist.");
}
void MenuManager::reshape(int w, int h) 
{
	topmenu.reshape(w, h);
	int diff = (height == -1) ? 0 : height - h;
	for (int i = 0; i < menus.size(); i++)
		menus[i]->reshape(w, diff);
	height = h;
}
bool MenuManager::assignShortcut(String id, char key, bool ignore)
{
	if (ignore or exists(id))
	{
		shortcuts[key] = id;
		return true;
	}
	return logError("failure to bind " + id);
}
String MenuManager::keyPressed(char key)
{
	//is a textbox active?
	textbox* T = NULL;
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->active)
			T = menus[i]->active;
	//log keystroke in textbox
	if (T) return T->log(key);
	//is shortcut? return if so
	return shortcuts[key];
}
void MenuManager::keySpecial(int key)
{
	//is a textbox active?
	textbox* T = NULL;
	for (int i = 0; i < menus.size(); i++)
		if (menus[i]->active)
			T = menus[i]->active;
	//log keystroke in textbox
	if (T) T->special(key);
}
String MenuManager::requestText(String id)
{
	element* e = getElem(id);
	if (e != NULL and e->Type() == element::_textbox)
		return ((textbox*)e)->contents;
	logError("element '" + id + "' is not valid.");
	return "";
}
bool MenuManager::requestCheck(String id)
{
	element* e = getElem(id);
	if (e != NULL and e->Type() == element::_checkbox)
		return ((checkbox*)e)->checked;
	return logError("element '" + id + "' is not valid.");
}
bool MenuManager::exists(String id)
{
	if (getElem(id) == NULL)
		return false;
	return true;
}
bool MenuManager::deleteElement(String id)
{
	//early out for blank id
	if (id == "")
		return logError("No id given.");
	String part[3];
	for (int i = 0; i < 3; i++)
	{
		part[i] = id.substr(0, id.find(':'));
		if (id.find(':') == std::string::npos) break;
		id = id.substr(id.find(':')+1);
	}
	if (part[0] == "menubar")
	{
		if (part[1] == "")
			return logError("Deleting topmenu not allowed.");
		for (int i = 0; i < topmenu.tabs.size(); i++)
		{
			if (part[1] == topmenu.tabs[i]->id)
			{
				if (part[2] == "")
				{
					delete topmenu.tabs[i];
					topmenu.tabs.erase(topmenu.tabs.begin()+i);
					return true;
				}
				for (int j = 0; j < topmenu.tabs[i]->buttons.size(); j++)
				{
					if (part[2] == topmenu.tabs[i]->buttons[j]->id)
					{
						delete topmenu.tabs[i]->buttons[j];
						topmenu.tabs[i]->buttons.erase(
							topmenu.tabs[i]->buttons.begin()+j);
						return true;
					}
				}
				return logError("Invalid button name when deleting " + id + ".");
			}
		}
		return logError("Invalid tab name when deleting " + id + ".");
	}
	for (int i = 0; i < menus.size(); i++)
	{
		if (part[0] == menus[i]->id)
		{
			if (part[1] == "")
			{
				delete menus[i];
				menus.erase(menus.begin()+i);
				return true;
			}
			for (int j = 0; j < menus[i]->elems.size(); j++)
			{
				if (part[1] == menus[i]->elems[j]->id)
				{
					delete menus[i]->elems[j];
					menus[i]->elems.erase(menus[i]->elems.begin()+j);
					return true;
				}
			}
			return logError("Menu element not found when deleting " + id + ".");
		}
	}
	return logError("Menu not found when deleting " + id + ".");
}
bool MenuManager::linkTextBox(String tid, String bid, bool ignore)
{
	element* box = getElem(tid);
	if (box == NULL or box->Type() != element::_textbox)
		return logError("Element '" + tid + "' is not a textbox.");
	if (ignore or (exists(bid) and getElem(bid)->Type() == element::_button))
	{
		((textbox*)box)->pseudoId = bid;
		return true;
	}
	return logError("Element '" + bid + "' not valid.");
}
element* MenuManager::getElem(String id)
{
	String part[3];
	for (int i = 0; i < 3; i++)
	{
		part[i] = id.substr(0, id.find(':'));
		if (id.find(':') == std::string::npos) break;
		id = id.substr(id.find(':')+1);
	}
	if (part[0] == "menubar")
	{
		if (part[1] == "")
			return &topmenu;
		for (int i = 0; i < topmenu.tabs.size(); i++)
		{
			if (part[1] == topmenu.tabs[i]->id)
			{
				if (part[2] == "")
					return topmenu.tabs[i];
				for (int j = 0; j < topmenu.tabs[i]->buttons.size(); j++)
					if (part[2] == topmenu.tabs[i]->buttons[j]->id)
						return topmenu.tabs[i]->buttons[j];
				logError(part[2]);
				return NULL;
			}
		}
		logError(part[1]);
		logError(part[2]);
		return NULL;
	}
	for (int i = 0; i < menus.size(); i++)
	{
		if (part[0] == menus[i]->id)
		{
			if (part[1] == "")
				return menus[i];
			for (int j = 0; j < menus[i]->elems.size(); j++)
				if (part[1] == menus[i]->elems[j]->id)
					return menus[i]->elems[j];
			return NULL;
		}
	}
	return NULL;
}

