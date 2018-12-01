#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGLLight)
{
	struct LightIndices
	{
		GMint32 Color;
		GMint32 Position;
		GMint32 Type;
		GMint32 AttenuationConstant;
		GMint32 AttenuationLinear;
		GMint32 AttenuationExp;
	};
	Vector<Vector<LightIndices>> lightIndices;
};

class GMGLLight : public GMLight
{
	GM_DECLARE_PRIVATE_NGO(GMGLLight)
	typedef GMLight Base;

public:
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getType() = 0;
};

class GMGLAmbientLight : public GMGLLight
{
	enum { LightType = 0 };

public:
	virtual int getType() override
	{
		return LightType;
	}
};

class GMGLDirectLight : public GMGLLight
{
	enum { LightType = 1 };

public:
	virtual int getType() override
	{
		return LightType;
	}
};


END_NS
#endif