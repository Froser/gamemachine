#ifndef __GMGBUFFER_H__
#define __GMGBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

struct IFramebuffers;
class GMGameObject;
class GMModel;
GM_PRIVATE_OBJECT(GMGBuffer)
{
	IFramebuffers* geometryFramebuffers = nullptr;
	IFramebuffers* materialFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMModel* quadModel = nullptr;
};

class GMGBuffer : public GMObject, public IGBuffer
{
	DECLARE_PRIVATE(GMGBuffer)

protected:
	~GMGBuffer();

public:
	virtual void init() override;

protected:
	GMGameObject* getQuad();

protected:
	virtual IFramebuffers* getGeometryFramebuffers();
	virtual IFramebuffers* getMaterialFramebuffers();
	virtual IFramebuffers* createGeometryFramebuffers() = 0;
	virtual IFramebuffers* createMaterialFramebuffers() = 0;

private:
	void createQuad();
};

END_NS
#endif