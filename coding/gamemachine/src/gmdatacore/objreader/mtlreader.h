#ifndef __MTLREADER_H__
#define __MTLREADER_H__
#include "common.h"
#include <fstream>
#include "mtlreaderprivate.h"
#include "gmdatacore/object.h"

BEGIN_NS

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