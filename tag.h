#ifndef TAG_H
#define TAG_H
#include <list>
#include <string>
#include <iostream>

namespace runner
{
	class tagmaker;
    class tag {
	friend class tagmaker;
        typedef std::string String;
        typedef std::istream IStream;
        typedef std::list<tag*> List;
        typedef std::list<tag*>::const_iterator ConstIter;
        typedef std::list<tag*>::iterator _Iter;
        typedef std::size_t Uint;
    public:
        tag();
        tag(const tag&);
        ~tag();

        tag& operator =(const tag&);

        const String& get_name() const
        { return _name; }
        const String& get_attribute() const
        { return _attribute; }
        const String& get_content() const
        { return _content; }
        Uint get_position() const
        { return _position; }

        Uint get_number_of_children() const
        { return _childTags.size(); }
        const tag* next_child() const;

        void clear();

        void read(IStream&);
		std::ostream& write(std::ostream& output) const;
    private:
        String _name;
        String _attribute;
        String _content;
        List _childTags;
        Uint _position;

        mutable ConstIter _iter;
    };
	
	class tagmaker {
	public:
		tagmaker(tag* ptag)
		{ _tag = ptag; }
		tagmaker(tag& rtag)
		{ _tag = &rtag; }
		tagmaker() {}
		void manage (tag* ptag)
		{ _tag = ptag; }
		void manage (tag& rtag)
		{ _tag = &rtag; }
		void set_name(std::string name)
		{ _tag->_name = name; }
		void set_attribute(std::string attr)
		{ _tag->_attribute = attr; }
		void set_content(std::string cont)
		{ _tag->_content = cont; }
		void add_child(tag* child)
		{ _tag->_childTags.push_back(child); }
		void add_child(tag& child)
		{ _tag->_childTags.push_back(&child); }
		void prepare()
		{ _tag->_iter = _tag->_childTags.begin(); }
	private:
		tag* _tag;
	};
}
#endif