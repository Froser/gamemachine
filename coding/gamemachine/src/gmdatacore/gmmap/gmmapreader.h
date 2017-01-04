#ifndef __GMMAP_READER_H__
#define __GMMAP_READER_H__
#include "common.h"
BEGIN_NS

struct GMMap;
class GMMapReader
{
public:
	static void readGMM(const char* filename, OUT GMMap** map);
};

END_NS
#endif