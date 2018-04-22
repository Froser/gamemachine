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

GM_PRIVATE_OBJECT(Demo_Collision)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
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
	DECLARE_PRIVATE_AND_BASE(Demo_Collision, DemoHandler)

public:
	using Base::Base;
	~Demo_Collision();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;
	virtual void onDeactivate() override;

private:
	void onWindowActivate();
	void pickUp(const gm::GMPhysicsRayTestResult& rayTestResult);
	void removePicked();
	void movePicked(const gm::GMPhysicsRayTestResult& rayTestResult);
};

#endif