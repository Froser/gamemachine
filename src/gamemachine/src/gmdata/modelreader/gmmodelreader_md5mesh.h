#ifndef __GMMODELREADER_MD5MESH_H__
#define __GMMODELREADER_MD5MESH_H__
#include <gmcommon.h>
#include "gmmodelreader.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMModelReader_MD5Mesh)
{

};

class GMModelReader_MD5Mesh : public GMObject, public IModelReader
{
	GM_DECLARE_PRIVATE(GMModelReader_MD5Mesh);

public:
	GMModelReader_MD5Mesh();
	~GMModelReader_MD5Mesh();

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models) override;
	virtual bool test(const GMBuffer& buffer) override;
};

END_NS
#endif