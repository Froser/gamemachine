#ifndef __CORE_UTIL_H__
#define __CORE_UTIL_H__
#include <gmecommon.h>
#include <gamemachine.h>

namespace core
{
	//! 表示描述一个平面的结构
	struct GMPlainDescription
	{
		GMfloat X; //!< 地形在x轴的起始位置。
		GMfloat Y; //!< 地形在z轴的起始位置。
		GMfloat Z; //!< 地形在z轴的起始位置。
		GMfloat length; //!< 地形在x轴的长度。
		GMfloat width; //!< 地形在y轴的长度。
		GMsize_t sliceM; //!< 地形在x轴的分块数。
		GMsize_t sliceN; //!< 地形在z轴的分块数。
		Array<GMfloat, 3> lineColor; //!< 线条颜色
	};

	void utCreatePlain(const GMPlainDescription&, REF GMSceneAsset&);
	GMModelAsset utGetFirstModelFromScene(GMSceneAsset);
	GMShader& utGetFirstShaderFromScene(GMSceneAsset);
}

#endif