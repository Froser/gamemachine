#ifndef __GMCRYPTOGRAPHIC_H__
#define __GMCRYPTOGRAPHIC_H__
#include <gmcommon.h>
BEGIN_NS

class GMCryptographic
{
public:
	enum Alogrithm
	{
		MD5,
	};

public:
	static void hash(const GMBuffer& in, Alogrithm method, OUT GMBuffer& out);
};

END_NS
#endif