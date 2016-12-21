#ifndef __OBJREADER_H__
#define __OBJREADER_H__
#include "common.h"
#include <fstream>
#include "objreaderprivate.h"

BEGIN_NS
class Object;
class ObjReader
{
	DEFINE_PRIVATE(ObjReader);

public:
	ObjReader();

public:
	void load(const char* filename, OUT Object** obj);

private:
	void parse(std::ifstream& file, OUT Object** obj);
};

END_NS
#endif