#ifndef __GMMODELREADER_MD5MESH_H__
#define __GMMODELREADER_MD5MESH_H__
#include <gmcommon.h>
#include "gmmodelreader.h"
#include "gmmodelreader_md5.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMModelReader_MD5Mesh)
{
	Vector<GMOwnedPtr<IMd5MeshHandler>> handlers;
	IMd5MeshHandler* nextHandler = nullptr;

	GMint MD5Version;
	GMString commandline;
	GMint numJoints;
	GMint numMeshes;
	GMint numWeights;
	Vector<GMModelReader_MD5Mesh_Joint> joints;
	Vector<GMModelReader_MD5Mesh_Mesh> meshes;
};

class GMModelReader_MD5Mesh : public GMModelReader_MD5
{
	GM_DECLARE_PRIVATE_AND_BASE(GMModelReader_MD5Mesh, GMModelReader_MD5)
	GM_DECLARE_PROPERTY(MD5Version, MD5Version, GMint)
	GM_DECLARE_PROPERTY(Commandline, commandline, GMString)
	GM_DECLARE_PROPERTY(NumJoints, numJoints, GMint)
	GM_DECLARE_PROPERTY(NumMeshes, numMeshes, GMint)

public:
	GMModelReader_MD5Mesh() = default;
	~GMModelReader_MD5Mesh() = default;

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models) override;
	virtual bool test(const GMBuffer& buffer) override;
	virtual Vector<GMOwnedPtr<IMd5MeshHandler>>& getHandlers() override;

public:
	inline void setNextHandler(IMd5MeshHandler* handler) GM_NOEXCEPT
	{
		D(d);
		d->nextHandler = handler;
	}

	void addJoint(GMModelReader_MD5Mesh_Joint&& joint)
	{
		D(d);
		d->joints.push_back(std::move(joint));
	}

	void addMesh(GMModelReader_MD5Mesh_Mesh&& mesh)
	{
		D(d);
		d->meshes.push_back(std::move(mesh));
	}
};

END_NS
#endif