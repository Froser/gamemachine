#ifndef __DEMO_COLLISION_H__
#define __DEMO_COLLISION_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

namespace gm
{
	class GMDiscreteDynamicsWorld;
	class GMPoint2PointConstraint;
}

GM_PRIVATE_OBJECT_ALIGNED(Demo_Collision)
{
	gm::GMGameObject* ground = nullptr;
	gm::GMPhysicsObject* firstPhyObj = nullptr;
	Vector<gm::GMGameObject*> cubes;
	gm::GMDiscreteDynamicsWorld* discreteWorld = nullptr;
	gm::GMRigidPhysicsObject* lastSelect = nullptr;
	GMVec3 lastColor;

	// Pick states
	gm::GMRigidPhysicsObject* pickedBody = nullptr;
	gm::GMPoint2PointConstraint* pickedConstraint = nullptr;
	gm::GMPhysicsActivationState pickedActivationState = gm::GMPhysicsActivationState::ActiveTag;
	GMVec3 oldPickingPos;
	GMVec3 hitPos;
	gm::GMfloat oldPickingDist;
};

class Demo_Collision : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Collision)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Collision(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;
	virtual void onDeactivate() override;

private:
	void onWindowActivate();
	void pickUp(const gm::GMPhysicsRayTestResult& rayTestResult);
	void removePicked();
	void movePicked(const gm::GMPhysicsRayTestResult& rayTestResult);

protected:
	const gm::GMString& getDescription() const override
	{
		static gm::GMString desc = L"左键选中物体。按住左键移动物体。";
		return desc;
	}

	virtual void createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset);
	virtual void createItems();
};

class Demo_Collision_Cone : public Demo_Collision
{
	using Demo_Collision::Demo_Collision;
	virtual void createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset);
};

class Demo_Collision_Cylinder : public Demo_Collision
{
	using Demo_Collision::Demo_Collision;
	virtual void createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset);
};

class Demo_Collision_Sphere : public Demo_Collision
{
	using Demo_Collision::Demo_Collision;
	virtual void createPhysicsShapeAsset(REF gm::GMPhysicsShapeAsset& asset);
};

class Demo_Collision_Model : public Demo_Collision
{
	using Demo_Collision::Demo_Collision;
	virtual void createItems();
};

class Demo_Collision_Chain : public Demo_Collision
{
	using Demo_Collision::Demo_Collision;
	virtual void createItems();
};

#endif