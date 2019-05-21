#ifndef __GMWAVEGAMEOBJECT_H__
#define __GMWAVEGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

#define WAVE_COUNT L"GM_Ext_Wave_WaveCount"
#define WAVE_DURATION L"GM_Ext_Wave_Duration"
#define WAVE_DESCRIPTION "GM_Ext_Wave_WaveDescriptions"
#define STEEPNESS "Steepness"
#define AMPLITUDE "Amplitude"
#define DIRECTION "Direction"
#define SPEED "Speed"
#define WAVELENGTH "WaveLength"

struct GMWaveGameObjectDescription
{
	GMfloat terrainX; //!< 地形在x轴的起始位置。
	GMfloat terrainZ; //!< 地形在z轴的起始位置。
	GMfloat terrainLength; //!< 地形在x轴的长度。
	GMfloat terrainWidth; //!< 地形在y轴的长度。
	GMfloat heightScaling; //!< 地形高度的缩放比例。
	GMint32 sliceM; //!< 地形在x轴的分块数。
	GMint32 sliceN; //!< 地形在z轴的分块数。
	GMfloat textureLength; //!< 一块地形纹理在x轴的长度。
	GMfloat textureHeight; //!< 一块地形纹理在z轴的长度。
	GMfloat textureScaleLength; //!< 纹理在x轴的缩放比，如2表示x方向显示2个单位的纹理。
	GMfloat textureScaleHeight; //!< 纹理在z轴的缩放比，如2表示z方向显示2个单位的纹理。
};

struct GMWaveDescription
{
	GMfloat steepness; // between 0 and 1
	GMfloat amplitude;
	GMfloat direction[3];
	GMfloat speed;
	GMfloat waveLength;
};

struct GMWaveDescriptionIndices
{
	GMint32 steepness;
	GMint32 amplitude;
	GMint32 direction;
	GMint32 speed;
	GMint32 waveLength;
};

struct GMWaveIndices
{
	GMint32 waveCount;
	GMint32 duration;
};

enum class GMWaveGameObjectHardwareAcceleration
{
	GPU,
	CPU,
};

GM_PRIVATE_OBJECT(GMWaveGameObject)
{
	GMWaveGameObjectDescription objectDescription;
	Vector<GMWaveDescription> waveDescriptions;
	GMVertices vertices;
	bool isPlaying = false;
	GMDuration duration = 0;
	GMModel* waveModel = nullptr;

	// GPU 相关变量
	Vector<Vector<GMWaveDescriptionIndices>> waveIndices;
	Vector<GMWaveIndices> globalIndices;

	GMWaveGameObjectHardwareAcceleration acceleration = GMWaveGameObjectHardwareAcceleration::GPU;
};

class GM_EXPORT GMWaveGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMWaveGameObject, GMGameObject)
	GM_DECLARE_PROPERTY(ObjectDescription, objectDescription)
	GM_DECLARE_PROPERTY(HandwareAcceleration, acceleration)

private:
	GMWaveGameObject() = default;

public:
	static void initShader(const IRenderContext* context);
	static GMWaveGameObject* create(const GMWaveGameObjectDescription& desc);

public:
	void setWaveDescriptions(Vector<GMWaveDescription> desc);
	void play();
	void stop();
	void update(GMDuration dt);

public:
	virtual void onRenderShader(GMModel* model, IShaderProgram* shaderProgram) const;
	virtual bool canDeferredRendering() override;

private:
	void updateEachVertex();

private:
	void setVertices(const GMVertices& vertices);
};

END_NS

#endif