#ifndef __GMPARTICLE_MODEL_COCOS2D__
#define __GMPARTICLE_MODEL_COCOS2D__
#include <gmcommon.h>
#include "../gmparticle.h"

BEGIN_NS

class GMParticleSystem_Cocos2D;
GM_PRIVATE_OBJECT(GMParticleModel_Cocos2D)
{
	GMOwnedPtr<GMGameObject> particleObject;
	GMModel* particleModel = nullptr;
	GMParticleSystem_Cocos2D* system = nullptr;
	bool GPUValid = true;
	GMComputeBufferHandle constantBuffer = 0;
	GMComputeBufferHandle particleBuffer = 0;
	GMComputeSRVHandle particleView = 0;
	GMComputeBufferHandle resultBuffer = 0;
	GMComputeUAVHandle resultView = 0;
	GMComputeBufferHandle resultBuffer_CPU = 0;
	bool particleSizeChanged = true;
	GMsize_t lastMaxSize = 0;
};

//! 表示一个2D粒子，是一个四边形
class GM_EXPORT GMParticleModel_Cocos2D : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleModel_Cocos2D)

public:
	enum BufferFlags
	{
		None = 0x00, // 考虑粒子的所有坐标分量
		IgnorePosZ = 0x01, // 计算粒子时不考虑粒子z坐标
	};

public:
	GMParticleModel_Cocos2D(GMParticleSystem_Cocos2D* system);
	~GMParticleModel_Cocos2D();

public:
	void render();

public:
	virtual void init();

protected:
	virtual GMGameObject* createGameObject(const IRenderContext* context);

	void update6Vertices(
		GMVertex* vertex,
		const GMVec3& centerPt,
		const GMVec2& halfExtents,
		const GMVec4& color,
		const GMQuat& quat,
		const GMVec3& lookDirection,
		GMfloat z = 0
	);

protected:
	virtual void updateData(void* dataPtr);
	virtual void CPUUpdate(void* dataPtr) = 0;
	virtual void GPUUpdate(IComputeShaderProgram*, void* dataPtr);
	virtual GMString getCode() = 0;

protected:
	virtual GMComputeBufferHandle prepareBuffers(IComputeShaderProgram*, BufferFlags = IgnorePosZ);

private:
	void initObjects();
	void disposeGPUHandles();
	void createBuffers(IComputeShaderProgram* shaderProgram);

public:
	static void setDefaultCode(const GMString& code);
};

class GMParticleModel_2D : public GMParticleModel_Cocos2D
{
public:
	using GMParticleModel_Cocos2D::GMParticleModel_Cocos2D;

protected:
	virtual void CPUUpdate(void* dataPtr) override;
	virtual GMString getCode() override;
};

class GMParticleModel_3D : public GMParticleModel_Cocos2D
{
public:
	using GMParticleModel_Cocos2D::GMParticleModel_Cocos2D;

protected:
	virtual void CPUUpdate(void* dataPtr) override;
	virtual GMString getCode() override;

protected:
	virtual GMComputeBufferHandle prepareBuffers(IComputeShaderProgram*, const IRenderContext* context, BufferFlags);
};

END_NS
#endif