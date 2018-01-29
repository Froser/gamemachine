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
	Vector<GMModel::DataType>* vertexData[(size_t)GMVertexDataType::EndOfVertexDataType];
	GMComPtr<ID3D11Buffer> buffers[(size_t)GMVertexDataType::EndOfVertexDataType];
	bool inited = false;
};

class GMDx11ModelPainter : public GMModelPainter
{
	DECLARE_PRIVATE_AND_BASE(GMDx11ModelPainter, GMModelPainter)

public:
	GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* obj);

public:
	virtual void transfer() override;
	virtual void draw(const GMGameObject* parent) override;
	virtual void dispose(GMMeshData* md) override;
	virtual void beginUpdateBuffer(GMMesh* mesh) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;
};

END_NS
#endif