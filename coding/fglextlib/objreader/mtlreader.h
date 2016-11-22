#ifndef __MTLREADER_H__
#include "common.h"
#include <fstream>
#include "mtlreader_private.h"

BEGIN_NS

class ObjReader;

class MtlReader
{
	DEFINE_PRIVATE(MtlReader);

public:
	MtlReader();

public:
	void load(const char* filename);
	const MaterialProperties& getProperties(const char* name);

private:
	void parse(std::ifstream& file);
};

END_NS
#endif