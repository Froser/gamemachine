#ifndef __GAMEOBJECT_P_H__
#define __GAMEOBJECT_P_H__
#include <gmcommon.h>
#include <linearmath.h>

BEGIN_NS

struct IComputeShaderProgram;

GM_PRIVATE_OBJECT_ALIGNED(GMGameObject)
{
	GMuint32 id = 0;
	GMGameObjectRenderPriority renderPriority = GMGameObjectRenderPriority::Normal; //!< 渲染优先级。优先级最高的对象将会在GMGameWorld中被优先渲染。
	GMOwnedPtr<GMPhysicsObject> physics;
	GMGameWorld* world = nullptr;
	const IRenderContext* context = nullptr;
	bool autoUpdateTransformMatrix = true;
	GMAsset asset;
	GMAnimationGameObjectHelper* helper = nullptr;

	GM_ALIGNED_16(struct) AABB
	{
		GMVec4 points[8];
	};
	GMGameObjectCullOption cullOption = GMGameObjectCullOption::NoCull;
	AlignedVector<AABB> cullAABB;
	GMCamera* cullCamera = nullptr;
	IComputeShaderProgram* cullShaderProgram = nullptr;
	GMComputeBufferHandle cullAABBsBuffer = 0;
	GMComputeBufferHandle cullGPUResultBuffer = 0;
	GMComputeBufferHandle cullCPUResultBuffer = 0;
	GMComputeBufferHandle cullFrustumBuffer = 0;
	GMComputeSRVHandle cullAABBsSRV = 0;
	GMComputeUAVHandle cullResultUAV = 0;
	GMsize_t cullSize = 0;
	bool cullGPUAccelerationValid = true;

	GM_ALIGNED_16(struct)
	{
		GMMat4 scaling = Identity<GMMat4>();
		GMMat4 translation = Identity<GMMat4>();
		GMQuat rotation = Identity<GMQuat>();
		GMMat4 transformMatrix = Identity<GMMat4>();
	} transforms;

	struct
	{
		bool visible = true;
	} attributes;

	struct
	{
		ITechnique* currentTechnique = nullptr;
	} drawContext;
};

END_NS
#endif
