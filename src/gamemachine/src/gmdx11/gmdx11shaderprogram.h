#ifndef __GMDX11SHADERPROGRAM_H__
#define __GMDX11SHADERPROGRAM_H__
#include <gmcommon.h>
#include <gmcom.h>
#include <gmdxincludes.h>
#include <gmgraphicengine.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11EffectShaderProgram)
{
	GMComPtr<ID3DX11Effect> effect;
	Vector<ID3DX11EffectVariable*> variables;
	GMAtomic<GMint32> nextVariableIndex;
};

class GM_EXPORT GMDx11EffectShaderProgram : public IShaderProgram
{
	GM_DECLARE_PRIVATE_NGO(GMDx11EffectShaderProgram)

public:
	GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect);
	~GMDx11EffectShaderProgram();

public:
	virtual void useProgram() override;
	virtual GMint32 getIndex(const GMString& name) override;
	virtual void setMatrix4(GMint32 index, const GMMat4& value) override;
	virtual void setVec4(GMint32 index, const GMFloat4&) override;
	virtual void setVec3(GMint32 index, const GMfloat value[3]) override;
	virtual void setInt(GMint32 index, GMint32 value) override;
	virtual void setFloat(GMint32 index, GMfloat value) override;
	virtual void setBool(GMint32 index, bool value) override;
	virtual bool setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type) override;

public:
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;
	virtual bool setInterface(GameMachineInterfaceID id, void* in) override;

private:
	ID3DX11EffectVectorVariable* getVectorVariable(GMint32 index);
	ID3DX11EffectMatrixVariable* getMatrixVariable(GMint32 index);
	ID3DX11EffectScalarVariable* getScalarVariable(GMint32 index);
	ID3DX11EffectInterfaceVariable* getInterfaceVariable(GMint32 index);
	ID3DX11EffectClassInstanceVariable* getInstanceVariable(GMint32 index);
};

class GMDx11GraphicEngine;
GM_PRIVATE_OBJECT_UNALIGNED(GMDx11ComputeShaderProgram)
{
	const IRenderContext* context = nullptr;
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11ComputeShader> shader;
	GMuint32 SRV_UAV_CB_count[3] = { 0 };
};

class GM_EXPORT GMDx11ComputeShaderProgram : public IComputeShaderProgram
{
	GM_DECLARE_PRIVATE_NGO(GMDx11ComputeShaderProgram)

public:
	GMDx11ComputeShaderProgram(const IRenderContext* context);

public:
	virtual void dispatch(GMint32 threadGroupCountX, GMint32 threadGroupCountY, GMint32 threadGroupCountZ) override;
	virtual void load(const GMString& path, const GMString& source, const GMString& entryPoint) override;
	virtual bool createReadOnlyBufferFrom(GMComputeBufferHandle bufferSrc, OUT GMComputeBufferHandle* bufferOut) override;
	virtual bool createBuffer(GMuint32 elementSize, GMuint32 count, void* pInitData, GMComputeBufferType type, OUT GMComputeBufferHandle* ppBufOut) override;
	virtual void release(GMComputeHandle) override;
	virtual bool createBufferShaderResourceView(GMComputeBufferHandle, OUT GMComputeSRVHandle*) override;
	virtual bool createBufferUnorderedAccessView(GMComputeBufferHandle, OUT GMComputeUAVHandle*) override;
	virtual void bindShaderResourceView(GMuint32, GMComputeSRVHandle*) override;
	virtual void bindUnorderedAccessView(GMuint32, GMComputeUAVHandle*) override;
	virtual void bindConstantBuffer(GMComputeBufferHandle handle) override;
	virtual void setBuffer(GMComputeBufferHandle, GMComputeBufferType type, void*, GMuint32) override;
	virtual void copyBuffer(GMComputeBufferHandle dest, GMComputeBufferHandle src) override;
	virtual void* mapBuffer(GMComputeBufferHandle handle) override;
	virtual void unmapBuffer(GMComputeBufferHandle handle) override;
	virtual bool canRead(GMComputeBufferHandle handle) override;
	virtual GMsize_t getBufferSize(GMComputeBufferType type, GMComputeBufferHandle handle) override;

public:
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;
	virtual bool setInterface(GameMachineInterfaceID id, void* in) override;

private:
	void cleanUp();
};

END_NS
#endif