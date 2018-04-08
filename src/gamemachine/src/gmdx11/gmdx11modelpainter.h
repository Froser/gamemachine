#ifndef __GMDX11MODELPAINTER_H__
#define __GMDX11MODELPAINTER_H__
#include <gmcommon.h>
#include <gmmodel.h>
#include <gmcom.h>
#include <gmdxincludes.h>

BEGIN_NS

class GMDx11GraphicEngine;
GM_PRIVATE_OBJECT(GMDx11ModelPainter)
{
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11Buffer> buffer;
	bool inited = false;
	D3D11_MAPPED_SUBRESOURCE* mappedSubResource = nullptr;
};

class GMDx11ModelPainter : public GMModelPainter
{
	DECLARE_PRIVATE_AND_BASE(GMDx11ModelPainter, GMModelPainter)

public:
	GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* obj);

public:
	virtual void transfer() override;
	virtual void draw(const GMGameObject* parent) override;
	virtual void dispose(GMModelBuffer* md) override;
	virtual void beginUpdateBuffer(GMModel* model) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;

private:
	void draw(IRenderer* renderer, GMModel* model);
};

END_NS
#endif