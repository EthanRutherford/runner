#include <iostream>

class Input{
	public:
		int get() const;
		void set();
		void waitfor();
		void wait() const;
	private:
		int	a, b;
};

class Menu{
	public:
		std::ostream& operator<<(std::ostream& stream) const;
		void getInput();
		Menu(){cursor = 0;};
		~Menu(){};
	protected:
		std::string* options;
		int size;
		int cursor;
		Input input;
		bool isUpdated;
	friend std::ostream& operator<<(std::ostream& stream, const Menu& menu);
};

class cMenu: public Menu{
	public:
		void execute();
		cMenu(std::string file);
		~cMenu(){};
};

class projectMenu: public cMenu{
	public:
		void execute();
		bool back() const {return Back;};
		projectMenu(std::string file);
		~projectMenu();
	private:
		void setup(std::string* stuff) const;
		std::string* commands;
		bool Back;
};

class startMenu: public cMenu{
	public:
		std::string execute();
		startMenu(std::string file);
		~startMenu();
	private:
		std::string newProject() const;
		void saveProject(std::string project, std::string* name, int files) const;
		void addProject(std::string& project) const;
		void deleteProject();
		std::string checkfor(std::string file) const;
};
