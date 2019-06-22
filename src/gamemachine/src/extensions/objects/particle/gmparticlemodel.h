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
class GM_EXPORT GMParticleModel : public GMObject, public IParticleModel
{
	GM_DECLARE_PRIVATE(GMParticleModel)

public:
	enum BufferFlags
	{
		None = 0x00, // 考虑粒子的所有坐标分量
		IgnorePosZ = 0x01, // 计算粒子时不考虑粒子z坐标
	};

public:
	GMParticleModel(GMParticleSystem* system);
	~GMParticleModel();

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
		const GMVec3& lookDirection,
		GMfloat z = 0
	);

protected:
	virtual void updateData(void* dataPtr);
	virtual void CPUUpdate(void* dataPtr) = 0;
	virtual void GPUUpdate(IComputeShaderProgram*, void* dataPtr);
	virtual GMString getCode() = 0;

protected:
	virtual GMComputeBufferHandle prepareBuffers(IComputeShaderProgram*, void* dataPtr, BufferFlags = IgnorePosZ);

private:
	void initObjects(const IRenderContext* context);
	void disposeGPUHandles();

public:
	static void setDefaultCode(const GMString& code);
};

class GMParticleModel_2D : public GMParticleModel
{
public:
	using GMParticleModel::GMParticleModel;

protected:
	virtual void CPUUpdate(void* dataPtr) override;
	virtual GMString getCode() override;
};

class GMParticleModel_3D : public GMParticleModel
{
public:
	using GMParticleModel::GMParticleModel;

protected:
	virtual void CPUUpdate(void* dataPtr) override;
	virtual GMString getCode() override;

protected:
	virtual GMComputeBufferHandle prepareBuffers(IComputeShaderProgram*, const IRenderContext* context, void* dataPtr, BufferFlags);
};

END_NS
#endif