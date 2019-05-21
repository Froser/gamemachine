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
	const IRenderContext* context = nullptr;
	IFramebuffers* geometryFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMGeometryPassingState state = GMGeometryPassingState::Done;
	GMGraphicEngine* engine = nullptr;
};

class GM_EXPORT GMGBuffer : public GMObject, public IGBuffer
{
	GM_DECLARE_PRIVATE(GMGBuffer)

protected:
	GMGBuffer(const IRenderContext* context);
	~GMGBuffer();

public:
	virtual void init() override;
	virtual void setGeometryPassingState(GMGeometryPassingState);
	virtual GMGeometryPassingState getGeometryPassingState();

public:
	virtual const IRenderContext* getContext();

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