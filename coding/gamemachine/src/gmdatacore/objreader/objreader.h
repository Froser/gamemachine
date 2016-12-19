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
	enum Mode
	{
		LoadAndDraw = ObjReaderPrivate::LoadAndDraw,
		LoadOnly
	};

public:
	ObjReader(Mode mode);

public:
	void load(const char* filename, Object* obj);

private:
	void parse(std::ifstream& file);

private:
	Mode m_mode;
};

END_NS
#endif