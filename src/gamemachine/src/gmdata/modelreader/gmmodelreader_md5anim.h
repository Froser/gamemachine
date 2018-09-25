#ifndef __GMMODELREADER_MD5ANIM_H__
#define __GMMODELREADER_MD5ANIM_H__
#include <gmcommon.h>
#include "gmmodelreader_md5.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMModelReader_MD5Anim)
{
	Vector<GMOwnedPtr<IMd5MeshHandler>> handlers;
	IMd5MeshHandler* nextHandler = nullptr;
	GMint32 MD5Version;
	GMString commandline;
	GMint32 numFrames;
	GMint32 numJoints;
	GMint32 frameRate;
	GMint32 numAnimatedComponents;
	GMModelReader_MD5Anim_Hierarchy hierarchy;
	AlignedVector<GMModelReader_MD5Anim_Bound> bounds;
	AlignedVector<GMModelReader_MD5Anim_Baseframe> baseframes;
	AlignedVector<GMModelReader_MD5Anim_Frame> frames;
};

class GMModelReader_MD5Anim : public GMModelReader_MD5
{
	GM_DECLARE_PRIVATE(GMModelReader_MD5Anim)
	GM_DECLARE_PROPERTY(MD5Version, MD5Version, GMint32)
	GM_DECLARE_PROPERTY(Commandline, commandline, GMString)
	GM_DECLARE_PROPERTY(NumFrames, numFrames, GMint32)
	GM_DECLARE_PROPERTY(NumJoints, numJoints, GMint32)
	GM_DECLARE_PROPERTY(FrameRate, frameRate, GMint32)
	GM_DECLARE_PROPERTY(NumAnimatedComponents, numAnimatedComponents, GMint32)

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
	virtual bool test(const GMBuffer& buffer) override;
	virtual Vector<GMOwnedPtr<IMd5MeshHandler>>& getHandlers() override;
	void addJointToHierarchy(GMModelReader_MD5Anim_Joint&& joint);
	void addBound(GMModelReader_MD5Anim_Bound&& bounds);
	void addBaseframe(GMModelReader_MD5Anim_Baseframe&& baseframe);
	void initFrames(GMint32 num);
	void setFrame(GMint32 index, GMModelReader_MD5Anim_Frame&& frame);

public:
	inline void setNextHandler(IMd5MeshHandler* handler) GM_NOEXCEPT
	{
		D(d);
		d->nextHandler = handler;
	}

private:
	void buildModel(GMModels* models);
};

END_NS
#endif