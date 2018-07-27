#ifndef __GMPARTICLE_MODEL__
#define __GMPARTICLE_MODEL__
#include <gmcommon.h>
#include "gmparticle.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMParticleModel_2D)
{
	GMOwnedPtr<GMGameObject> particleObject;
	GMOwnedPtr<GMModel> particleModel;
	GMParticleSystem* system = nullptr;
};

//! 表示一个2D粒子，是一个四边形
class GMParticleModel_2D : public GMObject, public IParticleModel
{
	GM_DECLARE_PRIVATE(GMParticleModel_2D)

public:
	GMParticleModel_2D(GMParticleSystem* system);

public:
	virtual void render(const IRenderContext* context) override;

private:
	GMGameObject* createGameObject(
		const IRenderContext* context
	);

	void updateData(
		const IRenderContext* context,
		void* dataPtr
	);

	void update6Vertices(
		GMVertex* vertex,
		const GMVec3& centerPt,
		const GMVec2& halfExtents,
		const GMVec4& color,
		GMfloat rotationRad,
		const GMVec3& rotationAxis,
		GMfloat z = 0
	);
};


END_NS
#endif