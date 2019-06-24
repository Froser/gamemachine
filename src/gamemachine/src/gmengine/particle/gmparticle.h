#ifndef __GMPARTICLE_H__
#define __GMPARTICLE_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

typedef const void *GMParticleDescription;

struct IParticleSystem;
struct IParticleEmitter;
GM_INTERFACE(IParticleSystemManager)
{
	virtual void addParticleSystem(AUTORELEASE IParticleSystem* ps) = 0;
	virtual void render() = 0;
	virtual void update(GMDuration dt) = 0;
};

GM_INTERFACE(IParticleSystem)
{
	virtual void update(GMDuration dt) = 0;
	virtual void render() = 0;
	virtual const IRenderContext* getContext() = 0;
	virtual void setParticleSystemManager(IParticleSystemManager* manager) = 0;
	virtual IParticleEmitter* getEmitter() = 0;
};

GM_INTERFACE(IParticleEmitter)
{
	virtual void emitParticles(GMDuration dt) = 0;
	virtual void emitOnce() = 0;
	virtual void update(GMDuration dt) = 0;
	virtual void startEmit() = 0;
	virtual void stopEmit() = 0;
};

GM_PRIVATE_OBJECT(GMParticleSystemManager)
{
	const IRenderContext* context;
	Vector<GMOwnedPtr<IParticleSystem>> particleSystems;
};

class GM_EXPORT GMParticleSystemManager : public GMObject, public IParticleSystemManager
{
	GM_DECLARE_PRIVATE(GMParticleSystemManager)

public:
	GMParticleSystemManager(const IRenderContext* context);

public:
	virtual void addParticleSystem(AUTORELEASE IParticleSystem* ps) override;
	virtual void render() override;
	virtual void update(GMDuration dt) override;
};

END_NS
#endif