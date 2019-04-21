#ifndef __GMPARTICLE_MODEL__
#define __GMPARTICLE_MODEL__
#include <gmcommon.h>
#include "gmparticle.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMParticleModel)
{
	GMOwnedPtr<GMGameObject> particleObject;
	GMModel* particleModel = nullptr;
	GMParticleSystem* system = nullptr;
};

//! 表示一个2D粒子，是一个四边形
class GMParticleModel : public GMObject, public IParticleModel
{
	GM_DECLARE_PRIVATE(GMParticleModel)

public:
	GMParticleModel(GMParticleSystem* system);

public:
	virtual void render(const IRenderContext* context) override;

protected:
	GMGameObject* createGameObject(
		const IRenderContext* context
	);

	void update6Vertices(
		GMVertex* vertex,
		const GMVec3& centerPt,
		const GMVec2& halfExtents,
		const GMVec4& color,
		const GMQuat& quat,
		const GMVec3& lookAt,
		GMfloat z = 0
	);

protected:
	virtual void updateData(const IRenderContext* context, void* dataPtr) = 0;
};

class GMParticleModel_2D : public GMParticleModel
{
public:
	using GMParticleModel::GMParticleModel;

	virtual void updateData(const IRenderContext* context, void* dataPtr) override;
};

class GMParticleModel_3D : public GMParticleModel
{
public:
	using GMParticleModel::GMParticleModel;

public:
	virtual void updateData(const IRenderContext* context, void* dataPtr) override;
};

END_NS
#endif