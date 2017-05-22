#ifndef __SKYGAMEOBJECT_H__
#define __SKYGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
BEGIN_NS

class SkyGameObject : public GameObject
{
public:
	SkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max);

private:
	void createSkyBox(OUT Object** obj);

private:
	linear_math::Vector3 m_min;
	linear_math::Vector3 m_max;
	Shader m_shader;
};

END_NS
#endif