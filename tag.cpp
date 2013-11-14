#include "tag.h"
using namespace std;
using namespace runner;

tag::tag()
{
    _position = 0;
    _iter = _childTags.begin();
}
tag::tag(const tag& obj)
{
    _name = obj._name;
    _attribute = obj._attribute;
    _content = obj._content;
    for (list<tag*>::const_iterator i = obj._childTags.begin(),e = obj._childTags.end();i!=e;i++)
        _childTags.push_back( new tag(**i) );
    _position = obj._position;
    _iter = _childTags.begin();
}
tag::~tag()
{
    for (_Iter i = _childTags.begin(), e = _childTags.end();i!=e;i++)
        delete *i;
}
tag& tag::operator =(const tag& obj)
{
    if (this != &obj)
    {
        _name = obj._name;
        _attribute = obj._attribute;
        _content = obj._content;
        for (list<tag*>::const_iterator i = obj._childTags.begin(),e = obj._childTags.end();i!=e;i++)
            _childTags.push_back( new tag(**i) );
        _position = obj._position;
        _iter = _childTags.begin();        
    }
    return *this;
}
const tag* tag::next_child() const
{
    if (_iter != _childTags.end())
        return *_iter++;
    _iter = _childTags.begin();
    return NULL;
}
void tag::clear()
{
    _name.clear();
    _attribute.clear();
    _content.clear();
	_childTags.clear();
}
void tag::read(istream& stream)
{
    char c;
    clear();
    // read until open tag
    do {
        c = stream.get();
        if (c == '<')
            break;
    } while ( stream.good() );
    // read tag name
    while ( stream.good() )
    {
        c = stream.get();
        if (c == '>')
            break;
        if (c == ' ')
        {
            // read tag attribute
            while ( stream.good() )
            {
                c = stream.get();
                if (c == '>')
                    break;
                if (c=='/' && stream.peek()=='>')
                {
                    stream.get(); // pop off >
                    return; // shorthand with attribute
                }
                _attribute.push_back(c);
            }
            break;
        }
        if (c=='/' && stream.peek()=='>')
        {
            stream.get(); // pop off >
            return; // shorthand
        }
        _name.push_back(c);
    }
    // read tag content
    while ( stream.good() )
    {
        c = stream.get();
        if (c == '<')
        {
            if (stream.peek() != '/')
            {
                // read sub (child) tag
                tag* child = new tag;
                stream.putback(c);
                child->read(stream);
                _childTags.push_back(child);
                child->_position = _content.length();
                continue;
            }
            else
            {
                string endTag;
                stream.get(); //pop off /
                while ( stream.good() )
                {
                    c = stream.get();
                    if (c == '>')
                        break;
                    endTag.push_back(c);
                }
                if (endTag != _name)
                    cerr << "Error: expected end-tag with name '" << _name << "'; found end tag with name '" << endTag << "'\n";
                break;
            }
        }
        _content.push_back(c);
    }
    _iter = _childTags.begin(); // set iterator to beginning of new child tags
    // normalize the tag-name by making it lower-case
    for (Uint i = 0;i<_name.length();i++)
        if (_name[i]>='A' && _name[i]<='Z')
            _name[i] = _name[i]-'A'+'a';
}
ostream& tag::write(ostream& output) const
{
	bool shorthand = true;
	output << "<" << _name;
	if (_attribute.length() > 0)
		output << _attribute;
	else if (get_number_of_children() > 0)
	{
		output << ">\n";
		const tag* pSubTag = next_child();
		while (pSubTag != NULL)
		{
			output << "\t";
			pSubTag->write(output);
			pSubTag = next_child();
		}
		shorthand = false;
	}
	else
	{
		output << ">" << _content;
		shorthand = false;
	}
	if (shorthand)
		output << "/>\n";
	else
		output << "</" << _name << ">\n";
	return output;
}