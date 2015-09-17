#ifndef XML_H
#define XML_H

#include <string>
#include <vector>
#include <fstream>

typedef std::string String;
typedef std::ofstream ofile;
typedef std::ifstream ifile;

struct tag{
	String name;
	String content;
	String attribute;
	bool shorthand;
	std::vector<tag> child;
};

struct Parser{
	bool read(String filename);
	std::vector<tag> tags;
	private: bool parse(tag& cur, ifile& text);
};

struct Writer{

	void write(String filename);
	std::vector<tag> tags;
};

#endif