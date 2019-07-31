#ifndef __GMEFFECTREADER_H__
#define __GMEFFECTREADER_H__
#include <gmcommon.h>
BEGIN_NS

class GMEffectGameObject;
GM_INTERFACE(IEffectObjectFactory)
{
	virtual GMEffectGameObject* createGameObject(GMAsset asset) = 0;
};

//! 读取一个特效模型的类。
struct GMEffectReader
{
	static bool loadEffect(const GMString& effectName, const IRenderContext* context, IEffectObjectFactory**);
};

END_NS
#endif