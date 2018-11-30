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
	GMShaderVariablesDesc desc;
	HashMap<GMString, ID3DX11EffectVectorVariable*, GMStringHashFunctor> vectors;
	HashMap<GMString, ID3DX11EffectMatrixVariable*, GMStringHashFunctor> matrices;
	HashMap<GMString, ID3DX11EffectScalarVariable*, GMStringHashFunctor> scalars;
	HashMap<GMString, ID3DX11EffectInterfaceVariable*, GMStringHashFunctor> interfaces;
	HashMap<GMString, ID3DX11EffectClassInstanceVariable*, GMStringHashFunctor> instances;
};

class GMDx11EffectShaderProgram : public IShaderProgram
{
	GM_DECLARE_PRIVATE_NGO(GMDx11EffectShaderProgram)

public:
	GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect);

public:
	virtual void useProgram() override;
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
	ID3DX11EffectVectorVariable* getVectorVariable(const GMString& name);
	ID3DX11EffectMatrixVariable* getMatrixVariable(const GMString& name);
	ID3DX11EffectScalarVariable* getScalarVariable(const GMString& name);
	ID3DX11EffectInterfaceVariable* getInterfaceVariable(const GMString& name);
	ID3DX11EffectClassInstanceVariable* getInstanceVariable(const GMString& name);
};

END_NS
#endif