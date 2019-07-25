#ifndef __GMDX11MODELPAINTER_H__
#define __GMDX11MODELPAINTER_H__
#include <gmcommon.h>
#include <gmmodel.h>
#include <gmcom.h>
#include <gmdxincludes.h>

BEGIN_NS

GM_PRIVATE_CLASS(GMDx11ModelDataProxy);
class GMDx11ModelDataProxy : public GMModelDataProxy
{
	GM_DECLARE_PRIVATE(GMDx11ModelDataProxy)
	GM_DECLARE_BASE(GMModelDataProxy)

public:
	GMDx11ModelDataProxy(const IRenderContext* context, GMModel* obj);
	~GMDx11ModelDataProxy();

public:
	virtual void transfer() override;
	virtual void dispose(GMModelBuffer* md) override;
	virtual void beginUpdateBuffer(GMModelBufferType type) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;
};

END_NS
#endif