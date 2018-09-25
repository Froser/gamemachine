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

	GMint32 MD5Version;
	GMString commandline;
	GMint32 numJoints;
	GMint32 numMeshes;
	GMint32 numWeights;
	AlignedVector<GMModelReader_MD5Mesh_Joint> joints;
	AlignedVector<GMModelReader_MD5Mesh_Mesh> meshes;
	HashMap<GMString, GMAsset, GMStringHashFunctor> shaders;
};

class GMModelReader_MD5Mesh : public GMModelReader_MD5
{
	GM_DECLARE_PRIVATE_AND_BASE(GMModelReader_MD5Mesh, GMModelReader_MD5)
	GM_DECLARE_PROPERTY(MD5Version, MD5Version, GMint32)
	GM_DECLARE_PROPERTY(Commandline, commandline, GMString)
	GM_DECLARE_PROPERTY(NumJoints, numJoints, GMint32)
	GM_DECLARE_PROPERTY(NumMeshes, numMeshes, GMint32)

public:
	GMModelReader_MD5Mesh() = default;
	~GMModelReader_MD5Mesh() = default;

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
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

	void initJoints(GMint32 num)
	{
		D(d);
		d->joints.reserve(num);
	}

	void initMeshes(GMint32 num)
	{
		D(d);
		d->meshes.reserve(num);
	}

private:
	void buildModel(const GMModelLoadSettings& settings, GMModels* models);
	void swapAll(GMModels* models);
};

END_NS
#endif