#ifndef __SKYGAMEOBJECT_H__
#define __SKYGAMEOBJECT_H__
#include "common.h"
#include "hallucinationgameobject.h"
BEGIN_NS

class SkyGameObject : public HallucinationGameObject
{
public:
	SkyGameObject(const Shader& shader, vmath::vec3& min, vmath::vec3 max);

private:
	void createSkyBox(OUT Object** obj);

private:
	vmath::vec3 m_min;
	vmath::vec3 m_max;
	Shader m_shader;
};

END_NS
#endif