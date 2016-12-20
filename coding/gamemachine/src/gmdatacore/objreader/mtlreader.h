#ifndef __MTLREADER_H__
#define __MTLREADER_H__
#include "common.h"
#include <fstream>
#include "mtlreaderprivate.h"
#include "gmdatacore/object.h"

BEGIN_NS

struct IObjReaderCallback;

class MtlReader
{
	DEFINE_PRIVATE(MtlReader);

public:
	MtlReader();

public:
	void load(const char* filename);
	const MaterialProperties& getProperties(const char* name);
	void setCallback(IObjReaderCallback* callback) { dataRef().setCallback(callback); }
	//TextureMap getTextureMap() { return dataRef().getTextureMap(); }

private:
	void parse(std::ifstream& file);
};

END_NS
#endif