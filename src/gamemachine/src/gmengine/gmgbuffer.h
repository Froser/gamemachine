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
	const GMContext* context = nullptr;
	IFramebuffers* geometryFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMModel* quadModel = nullptr;
	GMGeometryPassingState state = GMGeometryPassingState::Done;
	GMGraphicEngine* engine = nullptr;
};

class GMGBuffer : public GMObject, public IGBuffer, public IContext
{
	DECLARE_PRIVATE(GMGBuffer)

protected:
	GMGBuffer(const GMContext* context);
	~GMGBuffer();

public:
	virtual void init() override;
	virtual void setGeometryPassingState(GMGeometryPassingState);
	virtual GMGeometryPassingState getGeometryPassingState();

public:
	virtual const GMContext* getContext() override;

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