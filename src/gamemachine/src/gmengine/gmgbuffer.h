#ifndef __GMGBUFFER_H__
#define __GMGBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

struct IFramebuffers;
class GMGameObject;
class GMModel;
class GMGraphicEngine;
GM_PRIVATE_OBJECT(GMGBuffer)
{
	IFramebuffers* geometryFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMModel* quadModel = nullptr;
	GMGeometryPassingState state = GMGeometryPassingState::Done;
	GMGraphicEngine* engine = nullptr;
};

class GMGBuffer : public GMObject, public IGBuffer
{
	DECLARE_PRIVATE(GMGBuffer)

protected:
	GMGBuffer(GMGraphicEngine* engine);
	~GMGBuffer();

public:
	virtual void init() override;
	virtual void setGeometryPassingState(GMGeometryPassingState);
	virtual GMGeometryPassingState getGeometryPassingState();

protected:
	GMGameObject* getQuad();

protected:
	virtual IFramebuffers* getGeometryFramebuffers();
	virtual IFramebuffers* createGeometryFramebuffers() = 0;

private:
	void createQuad();
};

END_NS
#endif