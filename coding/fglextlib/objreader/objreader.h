#ifndef __OBJSCANNER_H__
#include "common.h"
#include <fstream>
#include "objreader_private.h"

BEGIN_NS

class ObjReader
{
	DEFINE_PRIVATE(ObjReader);

public:
	enum Mode
	{
		LoadAndDraw = ObjReader_Private::LoadAndDraw,
		LoadOnly
	};

public:
	ObjReader(Mode mode);

public:
	void load(const char* filename);
	void draw();

private:
	void parse(std::ifstream& file);

private:
	Mode m_mode;
};

END_NS
#endif