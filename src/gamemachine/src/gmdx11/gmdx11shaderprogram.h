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

class GMDx11EffectShaderProgram : public IShaderProgram
{
	GM_DECLARE_PRIVATE_NGO(GMDx11EffectShaderProgram)

public:
	GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect);

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

END_NS
#endif