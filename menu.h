//	!	HIGH PRIORITY
//	!	TODO: use subwindows (or working alternative)
//	+	MEDIUM PRIORITY
//	+	TODO: add radiobuttons
//	-	LOW PRIORITY
//	-	TODO: add spinners
//	?	UNDECIDED
//	?	TODO: multiline textboxes
//	?	TODO: hover-text
#ifndef MENU_H
#define MENU_H

#include <string>
#include <gl/gl.h>
#include <gl/freeglut.h>
#include <gl/glext.h>
#include <vector>
#include <map>

typedef std::string String;
class MenuManager;

//function for drawing text to screen
inline void drawText(String output, double x, double y){
	glRasterPos2d(x, y);
	glutBitmapString(GLUT_BITMAP_9_BY_15,(const unsigned char *)output.c_str());
}

//defines a rectangle
struct area{
	area() {}				//constructors for convenience
	area(int X, int Y, int W, int H) : x(X), y(Y), w(W), h(H) {}
	int x, y;				//lower-left position
	int w, h;				//size (width, height)
};

//base class
struct element{
	enum _Type {
		_button,
		_tab,
		_textbox,
		_textarea,
		_checkbox,
		_scrollbar,
		_Menu,
		_MenuBar
	};
	virtual void draw(int yoff = 0) {}	//this is a virtual class
	virtual _Type Type() {}				//get a type
	String name;						//display name
	String id;							//internal id
	bool clicked;						//is being clicked
	area a;								//rectangle definition
};

//data for a button's position and visual appearance
struct button : public element{
	_Type Type() {return _button;}
	void draw(int yoff = 0);	//opengl immediate mode currently
};

//data for tabs
struct tab : public element{
	~tab();
	_Type Type() {return _tab;}
	void draw(int yoff = 0);		//draw the tab
	bool open;						//is the tab open
	int buttonsize;					//uniform button width
	std::vector<button*> buttons;	//list of buttons in the menu
};

//data for a textbox
struct textbox : public element{
	_Type Type() {return _textbox;}
	void draw(int yoff = 0);		//draw the box
	String log(char key);			//log keystroke (returns true on enter key)
	void special(int key);			//log special keystroke
	String contents;				//contents
	int cursor;						//position of cursor
	int offset;						//offset from real cursor pos
	String pseudoId;
};

//data for text
struct textarea : public element{
	_Type Type() {return _textarea;}
	void draw(int yoff = 0);	//draw
	void prepare(int y, int h, int yoff);
	std::vector<String> lines;	//multiline text
	int start, end;				//start and end lines
};

//data for a checkbox
struct checkbox : public element{
	_Type Type() {return _checkbox;}
	void draw(int yoff = 0);	//draw checkbox
	bool checked;				//is checked
};

//data for a scrollbar
struct scrollbar : public element{
	_Type Type() {return _scrollbar;}
	void draw(int yoff = 0);	//draw scrollbar
	area b;						//inner bar
	int innerh;					//height of inner area
	int position;				//current position
	double ratio;				//ratio of bar to change
};

//Arbitrarily sized menu
class Menu : public element{
	friend class MenuManager;
	public:
		Menu() {active = NULL; bar = NULL;}
		Menu(String n, String i, int x, int y, int w, int h)
			{active = NULL; bar = NULL; a.x = x; a.y = y; a.w = w; a.h = h; name = n; id = i;}
		~Menu();
		void draw();							//draw the menu, and buttons
		_Type Type() {return _Menu;}			//get the type
		String click(int x, int y);				//register a click, return button id
		void release();							//mouse button released
		//fails if invalid button placement/name
		bool newButton(String n, String id, int x, int y);
		//fails if textbox is invalid
		bool newTextBox(String n, String id, int x, int y);
		//fails if text is invalid
		bool newText(String text, String id, int x, int y);
		//fails if checkbox invalid
		bool newCheckBox(String n, String id, bool check, int x, int y);
		bool setScrollBar(int innerh);
		bool inside(int X, int Y);				//test if point is within menu area
		void reshape(int width, int height);	//window reshape function
	private:
		std::vector<element*> elems;	//elements in menu
		scrollbar* bar;
		textbox* active;
};

//bar across top of window with options
class MenuBar : public element{
	friend class MenuManager;
	public:
		MenuBar() {nextX = 0;}
		MenuBar(String n, String i) {name = n; id = i; nextX = 0;}
		~MenuBar();
		void draw();								//draw the bar
		_Type Type() {return _Menu;}				//get the type
		String click(int x, int y);					//resolve a click
		void release();								//mb released
		void closeTabs();							//close the open tab
		bool newButton(String n, String id, String parent);	//add a button to a parent tab
		bool newTab(String n, String id);			//add new tab (drop down menu)
		bool inside(int x, int y);					//test if point is within menu area
		void reshape(int width, int height);		//window reshape function
	private:
		int nextX;									//position of next tab
		bool bttn;									//is true when a button is pressed
		std::vector<tab*> tabs;						//list of tabs
};

//interface for all menus, to consolidate functions
class MenuManager{
	public:
		MenuManager();
		~MenuManager();
		
		//initialize glut stuff
		void init();
		
		//the following are for callthroughs
		void DisplayFunc(void (*func)());
		void MouseFunc(void (*func)(int, int, int, int));
		void KeyboardFunc(void (*func)(unsigned char, int, int));
		void SpecialFunc(void (*func)(int, int, int));
		void ReshapeFunc(void (*func)(int, int));
		
		//register the callback
		void CallbackFunc(void (*func)(String));
		
		//draw all menus
		void draw();
		
		//register mouse (this function called internally)
		String mouse(int button, int state, int x, int y);
		//track mouse position (function used internally)
		void point(int x, int y, bool active = true);
		
		//add a new menu
		bool newMenu(String name, String id, int x, int y, int w, int h);
		
		//add a button to a parent menu/menubar tab
		bool newButton(String n, String id, String parent, int x=0, int y=0);
		
		//add a textbox to a parent menu
		bool newTextBox(String n, String id, String parent, int x, int y);
		
		//add a checkbox to a parent menu
		bool newCheckBox(String n, String id, String parent, bool check, int x, int y);
		
		//add text in a menu
		bool newText(String text, String id, String parent, int x, int y);
		
		//adds a new tab to the menubar
		bool newTab(String n, String id) {topmenu.newTab(n, id);}
		
		bool setScrollBar(String parent, int innerh);
		
		//maintains menubar's shape and position during window resize
		void reshape(int w, int h);
		
		//assign a keyboard shortcut to the button specified
		bool assignShortcut(String name, char key, bool ignore = false);
		
		//register keystrokes
		String keyPressed(char key);
		
		//register special keystrokes
		void keySpecial(int key);
		
		//get a textbox contents
		String requestText(String id);
		
		//get a checkbox state
		bool requestCheck(String id);
		
		//check if element exists
		bool exists(String id);
		
		//remove a menu or element by id. fails if no matching id
		bool deleteElement(String id);
		
		//add enterkey shortcut. fails if no match
		bool linkTextBox(String tid, String bid, bool ignore = false);
		
	private:
		//internal function for retrieving an element
		element* getElem(String id);
		//internal function for retrieving mouseovered element
		element* getMouseOvered();
		MenuBar topmenu;					//the menubar for the window
		std::vector<Menu*> menus;			//list of all menus	
		std::map<char,String> shortcuts;	//all keybindings
		String result;						//stores identifier of clicked button
		int height;							//height of window
		int mx, my;							//mouse position
};

#endif