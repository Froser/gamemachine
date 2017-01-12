#ifndef __SINGLEPRIMITIVEGAMEOBJECT_H__
#define __SINGLEPRIMITIVEGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
BEGIN_NS

class SinglePrimitiveGameObject : public GameObject
{
public:
	enum Type
	{
		Sphere,
	};

public:
	SinglePrimitiveGameObject(Type type, Material& material);

private:
	void createCoreShape();

private:
	Type m_type;
	Material m_material;
};

END_NS
#endif