#ifndef __GMMODELREADER_MD5ANIM_H__
#define __GMMODELREADER_MD5ANIM_H__
#include <gmcommon.h>
#include "gmmodelreader_md5.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMModelReader_MD5Anim)
{
	Vector<GMOwnedPtr<IMd5MeshHandler>> handlers;
	IMd5MeshHandler* nextHandler = nullptr;
	GMint MD5Version;
	GMString commandline;
};

class GMModelReader_MD5Anim : public GMModelReader_MD5
{
	GM_DECLARE_PRIVATE(GMModelReader_MD5Anim)
	GM_DECLARE_PROPERTY(MD5Version, MD5Version, GMint)
	GM_DECLARE_PROPERTY(Commandline, commandline, GMString)
};

END_NS
#endif