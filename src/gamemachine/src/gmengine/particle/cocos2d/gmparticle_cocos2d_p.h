#ifndef __GMPARTICLE_COCOS2D_P_H__
#define __GMPARTICLE_COCOS2D_P_H__
#include <gmcommon.h>
#include <linearmath.h>
BEGIN_NS

GM_ALIGNED_16(struct) GM_PRIVATE_NAME(GMParticle_Cocos2D)
{
	GMVec4 color = Zero<GMVec4>();
	GMVec4 deltaColor = Zero<GMVec4>();
	GMVec3 position = Zero<GMVec3>(); //xyz
	GMVec3 startPosition = Zero<GMVec3>(); //xyz
	GMVec3 changePosition = Zero<GMVec3>(); //xyz
	GMVec3 velocity = Zero<GMVec3>(); //xyz
	GMfloat size = 0;
	GMfloat currentSize = 0;
	GMfloat deltaSize = 0;
	GMfloat rotation = 0;
	GMfloat deltaRotation = 0;
	GMDuration remainingLife = 0;

	GMParticle_Cocos2D_GravityModeData gravityModeData;
	GMParticle_Cocos2D_RadiusModeData radiusModeData;
};

END_NS
#endif