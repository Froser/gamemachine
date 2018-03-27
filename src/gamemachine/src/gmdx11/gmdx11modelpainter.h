#ifndef __GMDX11MODELPAINTER_H__
#define __GMDX11MODELPAINTER_H__
#include <gmcommon.h>
#include <gmmodel.h>
#include <gmcom.h>
#include <gmdxincludes.h>

BEGIN_NS

// Dx11 顶点数据
struct GMDx11VertexData
{
	Array<GMfloat, 3> vertices;
	Array<GMfloat, 3> normals;
	Array<GMfloat, 2> texcoords;
	Array<GMfloat, 2> tangents;
	Array<GMfloat, 2> bitangents;
	Array<GMfloat, 2> lightmaps;
	Array<GMfloat, 4> color;
};

class GMDx11GraphicEngine;
GM_PRIVATE_OBJECT(GMDx11ModelPainter)
{
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11Buffer> buffer;
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

// IQueriable
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;

private:
	void packData(Vector<GMDx11VertexData>& packedData);
	void draw(IRenderer* renderer, GMComponent* component, GMMesh* mesh);
};

END_NS
#endif