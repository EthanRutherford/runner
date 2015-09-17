#include "xml.h"

bool Parser::read(String filename)
{
	ifile input(filename.c_str());
	if (input.fail())
		return false;
	char L;
	while (input.good())
	{
		L = input.get();
		if (L == '<')
		{
			tags.emplace_back();
			if (!parse(tags.back(), input))
				return false;
		}
	}
	return true;
}

bool Parser::parse(tag& cur, ifile& input)
{
	char L;
	cur.shorthand = false;
	while (input.good())	//get name
	{
		L = input.get();
		if (L == '>')
			break;
		if (L == ' ')		//get attribute (possibly)
		{
			if (input.peek() == ' ')
				continue;
			while (input.good())
			{
				L = input.get();
				if (L == '>')
					break;
				if (L == '/' and input.peek() == '>')
				{
					input.get();
					cur.shorthand = true;
					return true;
				}
				cur.attribute += L;
			}
			break;
		}
		if (L == '/' and input.peek() == '>')
		{
			input.get();
			cur.shorthand = true;
			return true;
		}
		cur.name += L;
	}
	while (input.good())	//get content
	{
		L = input.get();
		if (L == '<')		//get endtag, make sure match
		{
			if (input.peek() != '/')	//child tag
			{
				cur.child.emplace_back();
				cur.content += "@c";	//identifier for position of child
				parse(cur.child.back(), input);
				continue;
			}
			String endname;
			input.get();
			while (input.good())
			{
				L = input.get();
				if (L == '>')
					break;
				if (L == ' ')
					continue;
				endname += L;
			}
			if (endname != cur.name)
				return false;
			return true;
		}
		cur.content += L;
	}
	return true;
}

String writetag(tag& cur)
{
	String out = "";
	out += "<" + cur.name;								//print name
	if (cur.attribute.size())
		out += " " + cur.attribute;					//print attribute
	if (cur.shorthand)
		return out + "/>";								//shorthand
	out += ">";
	int childnum = 0;
	for (int i = 0; i < cur.content.size(); i++)
	{
		if (cur.content[i] == '@' and cur.content[i+1] == 'c')
		{
			out += writetag(cur.child[childnum++]);	//emplace children
			i++;
			continue;
		}
		out += cur.content[i];							//write content
	}
	for (int i = childnum; i < cur.child.size(); i++)		//print any remaining children
	{
		out += "\n\t";
		out += writetag(cur.child[childnum++]);		//emplace children
		if (childnum == cur.child.size())
			out += "\n";
	}
	out += "</" + cur.name + ">";						//endtag
	return out;
}

void Writer::write(String filename)
{
	ofile output(filename.c_str());
	for (int i = 0; i < tags.size(); i++)
		output << writetag(tags[i]) << '\n';
}

