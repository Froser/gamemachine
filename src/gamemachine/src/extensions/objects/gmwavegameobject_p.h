#ifndef __GMWAVEGAMEOBJECT_P_H__
#define __GMWAVEGAMEOBJECT_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMWaveGameObject)
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

END_NS

#endif