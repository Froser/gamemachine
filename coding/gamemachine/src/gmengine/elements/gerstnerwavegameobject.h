#ifndef __GERSTNERWAVEGAMEOBJECT_H__
#define __GERSTNERWAVEGAMEOBJECT_H__
#include "common.h"
#include "hallucinationgameobject.h"
#include "gmdatacore\object.h"

BEGIN_NS

struct GerstnerWaveProperties
{
	// 水波波长
	GMfloat waveLength;

	// 水波振幅
	GMfloat waveAmplitude;

	// 水波传播角度
	GMfloat waveDirection;

	// 水波速度
	GMfloat waveSpeed;

	// 开始坐标
	GMfloat startX;
	GMfloat startY;
};

struct GerstnerWavesProperties
{
	// 每个网格的X-Y坐标递增值
	GMfloat deltaX;
	GMfloat deltaY;

	// 水波条状数量
	GMint stripCount;

	// 水波长度
	GMint stripLength;

	// 水波数量
	GMfloat wavesCount;

	// 水波高度的缩放比例
	GMfloat waveHeightScale;

	// 每个水波属性
	GerstnerWaveProperties *waves;
};

inline void releaseGerstnerWavesProperties(GerstnerWavesProperties* props)
{
	if (props->waves)
		delete [] props->waves;
}

class GerstnerWaveGameObject : public HallucinationGameObject
{
public:
	static GerstnerWavesProperties& defaultProperties();

public:
	GerstnerWaveGameObject(const Material& material, const GerstnerWavesProperties& props);
	GerstnerWaveGameObject(const Material& material, GMfloat magnification, const GerstnerWavesProperties& props);
	virtual ~GerstnerWaveGameObject();

public:
	virtual void getReadyForRender(DrawingList& list) override;

public:
	void init();

private:
	Object* createCoreObject();
	void copyProperties(const GerstnerWavesProperties& props);
	void initSize();
	void initWave();
	void calcWave(ChildObject* obj, GMfloat elapsed);

private:
	Material m_material;
	GMfloat m_lastTick;
	GerstnerWavesProperties m_props;
	GMfloat m_magnification;

private:
	// 原始顶点所有浮点数的个数
	GMuint m_rawPointsLength;
	// 最终的顶点数据浮点数的个数
	GMuint m_dataLength;

	std::vector<GMfloat> m_rawStrips;
	std::vector<GMfloat> m_rawNormals;
	std::vector<GMfloat> m_uvs;
};

END_NS
#endif