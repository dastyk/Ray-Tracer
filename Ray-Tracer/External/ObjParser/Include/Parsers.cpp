#include "Parsers.h"
#include "ObjParser.h"
#include <fstream>

int ParseObj(const char * filename, ArfData::Data * data, ArfData::DataPointers * dataPointers)
{
	ObjParser::Interpreter parser;
	ifstream file(filename);

	if (file.is_open())
	{
		int res = parser.parse(&file, data, dataPointers);
		cout << "Parse complete. Result = " << res << endl;
		return res;
	}
	return 2;
}
struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

int ParseObj(const void * rawData, size_t size, ArfData::Data * data, ArfData::DataPointers * dataPointers)
{
	if (rawData != nullptr)
	{
		ObjParser::Interpreter parser;
		membuf buf((char*)rawData, (char*)rawData + size);
		istream stream(&buf);

		int res = parser.parse(&stream, data, dataPointers);
		cout << "Parse complete. Result = " << res << endl;

		return res;
	}
	return 2;
}
