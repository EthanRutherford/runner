#ifndef MENU_H
#define MENU_H
#include <iostream>
#include <vector>
#include "tag.h"

namespace runner
{
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
			void getInput();
			Menu(){cursor = 0;};
			~Menu(){};
		protected:
			std::vector<std::string> options;
			int size;
			int cursor;
			Input input;
			bool isUpdated;
		friend std::ostream& operator<<(std::ostream& stream, const Menu& menu);
	};

	class cMenu: public Menu{
		friend class Loader;
		public:
			virtual void execute();
			cMenu();
			~cMenu(){};
		protected:
			std::string changeto;
	};

	class projectMenu: public cMenu{
		public:
			void execute();
			projectMenu(tag proj);
			~projectMenu() {}
		private:
			void setup(std::string folder) const;
			std::vector<std::string> commands;
			std::vector<std::string> hfiles;
			std::vector<std::string> cppfiles;
	};

	class startMenu: public cMenu{
		public:
			void execute();
			startMenu(std::string file);
			~startMenu() {}
		private:
			std::string configfile;
			std::string newProject() const;
			void addProject(tag* child, int files) const;
			void deleteProject();
			std::string checkfor(std::string file) const;
	};

	class Loader{
		public:
			static Loader* get(std::string filename);
			void load();
			cMenu* menu() {return curMenu;}
			~Loader();
		private:
			static Loader* instance;
			Loader(std::string filename);
			Loader(const Loader&) {}
			std::string file;
			cMenu* curMenu;
	};
}
#endif