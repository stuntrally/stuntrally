#include "../propertycontainer.hpp"
#include <iostream>
#include <fstream>

#define TEST_EQUALS(a, b) if ((a) != (b)) std::cout << "FAIL: " << a << " != " << b << " at line " << __LINE__ << std::endl

int main(int argc, char* argv[])
{
	PropertyContainer writer;
	writer
		.add("Int", 4)
		.add("Byte", (char)127)
		.add("Double", 666.6)
		.add("String", std::string("Test string"))
		.add("Data string", std::string("\n\t\r\0 x"))
		.add("C string", std::string("Test c-string").c_str())
		.add("Float", 0.5f)
		.add("Boolean false", false)
		.add("Boolean true", true);

	PropertyContainer reader;
	reader.deserialize(writer.serialize(), writer.size());
	TEST_EQUALS(reader.count(), 9);
	TEST_EQUALS(reader.get<int>("Int"), 4);
	TEST_EQUALS(reader.get<char>("Byte"), 127);
	TEST_EQUALS(reader.get<double>("Double"), 666.6);
	TEST_EQUALS(reader.get<std::string>("String"), "Test string");
	TEST_EQUALS(reader.get<std::string>("Data string"), "\n\t\r\0 x");
	TEST_EQUALS(reader.get<std::string>("C string"), "Test c-string");
	TEST_EQUALS(reader.get<float>("Float"), 0.5f);
	TEST_EQUALS(reader.get<bool>("Boolean false"), false);
	TEST_EQUALS(reader.get<bool>("Boolean true"), true);

	return 0;
}
