#ifndef __SKYGAMEOBJECT_H__
#define __SKYGAMEOBJECT_H__
#include "common.h"
#include "hallucinationgameobject.h"
BEGIN_NS

class SkyGameObject : public HallucinationGameObject
{
public:
	SkyGameObject(ITexture* texture, vmath::vec3& min, vmath::vec3 max);

private:
	void createSkyBox(OUT Object** obj);

private:
	ITexture* m_texture;
	vmath::vec3 m_min;
	vmath::vec3 m_max;
};

END_NS
#endif