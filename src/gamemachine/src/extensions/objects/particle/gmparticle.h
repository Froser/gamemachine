#ifndef __GMPARTICLE_H__
#define __GMPARTICLE_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

typedef const void *GMParticleDescription;
typedef void* GMParticle;

GM_INTERFACE(IParticleSystemManager)
{
	virtual void addParticleSystem(AUTORELEASE IParticleSystem* ps) = 0;
	virtual void render() = 0;
	virtual void update(GMDuration dt) = 0;
};

GM_INTERFACE(IParticleModel)
{
	virtual void render() = 0;
};

GM_INTERFACE(IParticleSystem)
{
	virtual void update(GMDuration dt) = 0;
	virtual void render() = 0;
	virtual const IRenderContext* getContext() = 0;
	virtual IParticleModel* createParticleModel(GMParticleDescription desc) = 0;
};

GM_INTERFACE(IParticleEmitter)
{
	virtual void setDescription(GMParticleDescription desc) = 0;
	virtual void setParticleEffect(IParticleEffect* effect) = 0;
	virtual void addParticle() = 0;
	virtual void emitParticles(GMDuration dt) = 0;
	virtual void emitOnce() = 0;
	virtual void update(GMDuration dt) = 0;
	virtual void startEmit() = 0;
	virtual void stopEmit() = 0;
	virtual IParticleSystem* getParticleSystem() = 0;
};

GM_INTERFACE(IParticleEffect)
{
	virtual void setParticleDescription(GMParticleDescription desc) = 0;
	virtual void initParticle(IParticleEmitter* emitter, GMParticle particle) = 0;
	virtual void update(IParticleEmitter* emitter, GMDuration dt) = 0;
};

END_NS
#endif