#ifndef __GMWAVEGAMEOBJECT_H__
#define __GMWAVEGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

struct GMWaveGameObjectDescription
{
	GMfloat terrainX; //!< 地形在x轴的起始位置。
	GMfloat terrainZ; //!< 地形在z轴的起始位置。
	GMfloat terrainLength; //!< 地形在x轴的长度。
	GMfloat terrainWidth; //!< 地形在y轴的长度。
	GMfloat heightScaling; //!< 地形高度的缩放比例。
	GMsize_t sliceM; //!< 地形在x轴的分块数。
	GMsize_t sliceN; //!< 地形在z轴的分块数。
	GMfloat textureLength; //!< 一块地形纹理在x轴的长度。
	GMfloat textureHeight; //!< 一块地形纹理在z轴的长度。
};

struct GMWaveDescription
{
	GMfloat Q; // steepness
	GMfloat A; // amplitude
	GMfloat D; // wavelength
	GMfloat theta; // direction
};

GM_PRIVATE_OBJECT(GMWaveGameObject)
{
	Vector<GMWaveDescription> waveDescriptions;
	bool isPlaying = false;
	GMDuration duration = 0;
};

class GMWaveGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMWaveGameObject, GMGameObject)

private:
	GMWaveGameObject() = default;

public:
	static GMWaveGameObject* create(const GMWaveGameObjectDescription& desc);

public:
	void setWaveDescriptions(Vector<GMWaveDescription> desc);
	void play();
	void update(GMDuration dt);

private:
	void updateEachVertex(GMDuration dt);
};

END_NS
#endif