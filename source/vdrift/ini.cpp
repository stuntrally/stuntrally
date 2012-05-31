#include "pch.h"
/*
 * INI file structure:
 *
 * # comment
 * key1 = value1
 *
 * [key2]
 * key5 = value5
 *
 * [key2.key3]
 * key4 = value4
 *
 */
#include "ptree.h"

struct ini
{
	std::istream & in;
	PTree & root;
	const file_open * fopen;
	PTree cache;

	ini(std::istream & in, PTree & root, const file_open * fopen = 0) : in(in), root(root), fopen(fopen)
	{
		// Constructor.
	}

	void read()
	{
		read(root);
	}

	void read(PTree & node)
	{
		std::string line, name;
		while (in.good())
		{
			std::getline(in, line, '\n');
			if (line.empty())
			{
				continue;
			}

			size_t begin = line.find_first_not_of(" \t[");
			size_t end = line.find_first_of(";#]\r", begin);
			if (begin >= end)
			{
				continue;
			}

			size_t next = line.find("=", begin);
			if (next >= end)
			{
				// New node.
				next = line.find_last_not_of(" \t\r]", end);
				name = line.substr(begin, next);
				read(root.set(name, PTree()));
				continue;
			}

			size_t next2 = line.find_first_not_of(" \t\r", next+1);
			next = line.find_last_not_of(" \t", next-1);
			if (next2 >= end)
				continue;

			name = line.substr(begin, next+1);
			if (!fopen || line.at(next2) != '&')
			{
				// New property.
				std::string value = line.substr(next2, end-next2);
				node.set(name, value);
				continue;
			}

			// Value is a reference.
			std::string value = line.substr(next2+1, end-next2-1);
			const PTree * ref_ptr;
			if (root.get(value, ref_ptr) || cache.get(value, ref_ptr))
			{
				node.set(name, *ref_ptr);
				continue;
			}

			// Load external reference.
			PTree ref;
			read_ini(value, *fopen, ref);
			cache.set(value, ref);
			node.set(name, ref);
		}
	}
};

static void write_ini(const PTree & p, std::ostream & out, std::string key_name)
{
	for (PTree::const_iterator i = p.begin(), e = p.end(); i != e; ++i)
	{
		if (i->second.size() == 0)
		{
			out << i->first << " = " << i->second.value() << "\n";
		}
	}
	out << "\n";

	for (PTree::const_iterator i = p.begin(), e = p.end(); i != e; ++i)
	{
		if (i->second.size() > 0)
		{
			out << "[" << key_name + i->first << "]\n";
			write_ini(i->second, out, key_name + i->first + ".");
		}
	}
}

void read_ini(std::istream & in, PTree & p)
{
	ini reader(in, p);
	reader.read();
}

bool read_ini(const std::string & file_name, const file_open & fopen, PTree & p)
{
	p.value() = file_name;
	std::istream * in = fopen(file_name);
	if (in->good())
	{
		ini reader(*in, p, &fopen);
		reader.read();
		delete in;
		return true;
	}
	delete in;
	return false;
}

void write_ini(const PTree & p, std::ostream & out)
{
	write_ini(p, out, std::string());
}
