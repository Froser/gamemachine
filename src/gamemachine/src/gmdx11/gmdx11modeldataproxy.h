#ifndef __GMDX11MODELPAINTER_H__
#define __GMDX11MODELPAINTER_H__
#include <gmcommon.h>
#include <gmmodel.h>
#include <gmcom.h>
#include <gmdxincludes.h>

BEGIN_NS

class GMDx11GraphicEngine;
GM_PRIVATE_OBJECT(GMDx11ModelDataProxy)
{
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11Buffer> vertexBuffer;
	GMComPtr<ID3D11Buffer> indexBuffer;
	bool inited = false;
	D3D11_MAPPED_SUBRESOURCE* mappedSubResource = nullptr;
};

class GMDx11ModelDataProxy : public GMModelDataProxy
{
	DECLARE_PRIVATE_AND_BASE(GMDx11ModelDataProxy, GMModelDataProxy)

public:
	GMDx11ModelDataProxy(GMDx11GraphicEngine* engine, GMModel* obj);

public:
	virtual void transfer() override;
	virtual void dispose(GMModelBuffer* md) override;
	virtual void beginUpdateBuffer() override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;
};

END_NS
#endif