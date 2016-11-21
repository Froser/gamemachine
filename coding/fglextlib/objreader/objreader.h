#ifndef __OBJSCANNER_H__
#include "common.h"
#include <fstream>
#include "objreader_private.h"

BEGIN_NS

class ObjReader
{
	DEFINE_PRIVATE(ObjReader);

public:
	ObjReader();

public:
	void load(const char* filename);

private:
	void parse(std::ifstream& file);
};

END_NS
#endif