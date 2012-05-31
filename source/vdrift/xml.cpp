#include "pch.h"
/*
 * XML file format:
 *
 * <!-- comment -->
 * <key1>value1</key1>
 * <key2>
 *    <key3>
 *        <key4>value4</key4>
 *    </key3>
 *    <key5>value5</key5>
 * </key2>
 *
 */
#include "ptree.h"

static void read_xml(std::istream & in, PTree & p, std::string key)
{
	std::string line, escape("/"+p.value());
	while (in.good())
	{
		std::getline(in, line, '\n');
		if (line.empty())
		{
			continue;
		}

		size_t begin = line.find_first_not_of(" \t\n<");
		size_t end = line.length();
		if (begin >= end || line[begin] == '!')
		{
			continue;
		}

		line = line.substr(begin, end);

		if (line.find(escape) == 0)
		{
			break;
		}

		if (key.length() == 0)
		{
			end = line.find(" ");
			key = line.substr(0, end);
			continue;
		}

		size_t next = line.find("</"+key);
		if (next < end)
		{
			std::string value = line.substr(0, next);
			p.set(key, value);
		}
		else
		{
			end = line.find(" ");
			std::string child_key = line.substr(0, end);
			read_xml(in, p.set(key, PTree()), child_key);
		}
		key.clear();
	}
}

static void write_xml(const PTree & p, std::ostream & out, std::string indent)
{
	for (PTree::const_iterator i = p.begin(), e = p.end(); i != e; ++i)
	{
		if (i->second.size() == 0)
		{
			out << indent << "<" << i->first << ">" << i->second.value() << "</" << i->first << ">\n";
			write_xml(i->second, out, indent+"\t");
		}
		else
		{
			out << indent << "<" << i->first << ">\n";
			write_xml(i->second, out, indent+"\t");
			out << indent << "</" << i->first << ">\n";
		}
	}
}

void read_xml(std::istream & in, PTree & p)
{
	read_xml(in, p, std::string());
}

void write_xml(const PTree & p, std::ostream & out)
{
	write_xml(p, out, std::string());
}
