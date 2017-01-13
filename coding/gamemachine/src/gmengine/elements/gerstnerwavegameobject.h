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

	// 水波速度
	GMfloat waveSpeed;

	// 水波传播角度
	GMfloat waveDirection;

	// 开始坐标
	GMfloat startX;
	GMfloat startY;
};

struct GerstnerWavesProperties
{
	// 总大小
	GMuint length;
	GMuint width;

	// 水波条状数量
	GMuint stripCount;

	// 水波长度
	GMfloat stripLength;

	// 水波数量
	GMfloat wavesCount;

	// 水波高度的缩放比例
	GMfloat waveHeightScale;

	// 每个水波属性
	GerstnerWaveProperties *waves;
};

class GerstnerWaveGameObject : public HallucinationGameObject
{
public:
	GerstnerWaveGameObject(const Material& material, const GerstnerWavesProperties& props);

public:
	virtual void getReadyForRender(DrawingList& list) override;

private:
	Object* createCoreObject();
	void initAll();
	void initSize();
	void initWave();
	void calcWave();
	void calcWave(Object* obj, GMfloat elapsed);

private:
	Material m_material;
	GMfloat m_lastTick;
	GerstnerWavesProperties m_props;

private:
	// 原始顶点所有浮点数的个数
	GMuint m_rawPointsLength;
	// 最终的顶点数据浮点数的个数
	GMuint m_dataLength;

	std::vector<GMfloat> m_rawStrips;
	std::vector<GMfloat> m_rawNormals;
};

END_NS
#endif