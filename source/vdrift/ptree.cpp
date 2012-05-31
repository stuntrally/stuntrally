#include "pch.h"
#include "ptree.h"
#include "unittest.h"

#include <fstream>

file_open_basic::file_open_basic(const std::string & path, const std::string & path_alt) :
	path(path), path_alt(path_alt)
{
	// ctor
}

std::istream * file_open_basic::operator()(const std::string & name) const
{
	std::ifstream * file = new std::ifstream();

	// external config in track path
	std::string file_path = path + "/" + name;
	file->open(file_path.c_str());
	if (!file->good())
	{
		// external config in shared path
		file_path = path_alt + "/" + name;
		file->close();
		file->clear();
		file->open(file_path.c_str());
	}
	return file;
}

QT_TEST(ptree)
{
	std::stringstream err;
	std::string str;

	PTree ptree("test.cfg");
	PTree & root = ptree.set("root", PTree());
	root.set("foo", "123");
	root.set("bar", 456);

	PTree & child = root.set("child", PTree());
	child.set("lorem", true);
	child.set("ipsum", 7.89);

	const PTree * troot(0);
	ptree.get("root", troot, err);
	QT_CHECK_EQUAL(err.str(), "");

	troot->get("foo", str, err);
	QT_CHECK_EQUAL(str, "123");

	const PTree * tchild = 0;
	troot->get("child", tchild, err);
	QT_CHECK_EQUAL(err.str(), "");

	QT_CHECK(troot->get("bla", str, err) == false);
	std::string err_test = "test.cfg.root.bla not found.\n";
	QT_CHECK_EQUAL(err.str(), err_test);

	ptree.get("root.child.ipsum", str, err);
	QT_CHECK_EQUAL(str, "7.89");

	int i = 0;
	troot->get("bar", i, err);
	QT_CHECK_EQUAL(i, 456);

	bool b = false;
	tchild->get("lorem", b, err);
	QT_CHECK_EQUAL(b, true);

	PTree initree;
	std::stringstream ini, ini_test;
	write_ini(ptree, ini);
	read_ini(ini, initree);
	write_ini(initree, ini_test);
	QT_CHECK_EQUAL(ini.str(), ini_test.str());

	PTree inftree;
	std::stringstream inf, inf_test;
	write_inf(ptree, inf);
	read_inf(inf, inftree);
	write_inf(inftree, inf_test);
	QT_CHECK_EQUAL(inf.str(), inf_test.str());

	PTree xmltree;
	std::stringstream xml, xml_test;
	write_xml(ptree, xml);
	read_xml(xml, xmltree);
	write_xml(xmltree, xml_test);
	QT_CHECK_EQUAL(xml.str(), xml_test.str());
}
