#include "stdafx.h"
#include "gmdemogameworld.h"
#include "foundation/gamemachine.h"

GMDemoGameWorld::GMDemoGameWorld()
{
}

GMDemoGameWorld::~GMDemoGameWorld()
{
	D(d);
	for (auto& object : d->renderList)
	{
		delete object.second;
	}
}

void GMDemoGameWorld::renderGameWorld()
{
	D(d);
	Base::renderGameWorld();

	IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
	for (auto& object : d->renderList)
	{
		engine->drawObject(object.second);
	}
}

GMPhysicsWorld* GMDemoGameWorld::physicsWorld()
{
	return nullptr;
}

bool GMDemoGameWorld::appendObject(const GMString& name, GMGameObject* obj)
{
	D(d);
	auto& r = d->renderList.find(name);
	if (r != d->renderList.end())
		return false;
	d->renderList[name] = obj;
	return true;
}

void GMDemoGameWorld::createCube(GMfloat extents[3], OUT GMGameObject** obj)
{
	static constexpr GMfloat v[24] = {
		1, -1, 1,
		1, -1, -1,
		-1, -1, 1,
		-1, -1, -1,
		1, 1, 1,
		1, 1, -1,
		-1, 1, 1,
		-1, 1, -1,
	};
	static constexpr GMint indices[] = {
		0, 2, 1,
		2, 3, 1,
		4, 5, 6,
		6, 5, 7,
		0, 1, 4,
		1, 5, 4,
		2, 6, 3,
		3, 6, 7,
		0, 4, 2,
		2, 4, 6,
		1, 3, 5,
		3, 7, 5,
	};

	static constexpr GMint borderIndices[] = {
		0, 2, 3, 1,
		4, 5, 7, 6,
		0, 1, 5, 4,
		2, 6, 7, 3,
		0, 4, 2, 6,
		1, 3, 7, 5,
	};

	Object* coreObj = new Object();

	// 实体
	GMfloat t[24];
	for (GMint i = 0; i < 24; i++)
	{
		t[i] = extents[i % 3] * v[i];
	}

	{
		GMMesh* body = new GMMesh();
		body->setArrangementMode(GMArrangementMode::Triangle_Strip);

		Component* component = new Component(body);

		linear_math::Vector3 normal;
		for (GMint i = 0; i < 12; i++)
		{
			component->beginFace();
			for (GMint j = 0; j < 3; j++) // j表示面的一个顶点
			{
				GMint idx = i * 3 + j; //顶点的开始
				GMint idx_next = i * 3 + (j + 1) % 3;
				GMint idx_prev = i * 3 + (j + 2) % 3;
				linear_math::Vector3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
				linear_math::Vector3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
					vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
				linear_math::Vector3 normal = linear_math::cross(vertex - vertex_prev, vertex_next - vertex);
				normal = linear_math::normalize(normal);

				component->vertex(vertex[0], vertex[1], vertex[2]);
				component->normal(normal[0], normal[1], normal[2]);
			}
			component->endFace();
		}
		body->appendComponent(component);
		coreObj->append(body);
	}

	GMGameObject* gameObject = new GMGameObject(coreObj);
	*obj = gameObject;
	GameMachine::instance().initObjectPainter(gameObject);
}

GMGameObject* GMDemoGameWorld::getGameObject(const GMString& name)
{
	D(d);
	auto target = d->renderList.find(name);
	if (target == d->renderList.end())
		return nullptr;
	return target->second;
}