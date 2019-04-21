#ifndef __GMPARTICLEEFFECT_H__
#define __GMPARTICLEEFFECT_H__
#include <gmcommon.h>
#include "gmparticle.h"
BEGIN_NS

class GMGravityParticleEffect : public GMParticleEffect
{
public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;
	virtual void update(GMParticleEmitter* emitter, GMDuration dt) override;
};

class GMRadialParticleEffect : public GMParticleEffect
{
public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;
	virtual void update(GMParticleEmitter* emitter, GMDuration dt) override;
};


END_NS
#endif