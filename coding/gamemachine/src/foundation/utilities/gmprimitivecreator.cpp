#include "stdafx.h"
#include "gmprimitivecreator.h"

void GMPrimitiveCreator::createCube(GMfloat extents[3], OUT GMModel** obj, GMMeshType type)
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

	GMModel* coreObj = new GMModel();

	// 实体
	GMfloat t[24];
	for (GMint i = 0; i < 24; i++)
	{
		t[i] = extents[i % 3] * v[i];
	}

	{
		GMMesh* body = new GMMesh();
		body->setArrangementMode(GMArrangementMode::Triangle_Strip);
		body->setType(type);

		GMComponent* component = new GMComponent(body);

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
				component->color(1.f, 1.f, 1.f);
			}
			component->endFace();
		}
		body->appendComponent(component);
		coreObj->append(body);
	}

	*obj = coreObj;
}

void GMPrimitiveCreator::createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, GMMeshType type)
{
	static constexpr GMfloat v[] = {
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
	};

	static constexpr GMint indices[] = {
		0, 1, 3,
		2, 3, 1,
	};

	GMModel* coreObj = new GMModel();

	// 实体
	GMfloat t[12];
	for (GMint i = 0; i < 12; i++)
	{
		t[i] = (extents[i % 3] + position[i % 3]) * v[i];
	}

	{
		GMMesh* body = new GMMesh();
		body->setArrangementMode(GMArrangementMode::Triangle_Strip);
		body->setType(type);

		GMComponent* component = new GMComponent(body);

		linear_math::Vector3 normal;

		for (GMint i = 0; i < 2; i++)
		{
			component->beginFace();
			for (GMint j = 0; j < 3; j++) // j表示面的一个顶点
			{
				GMint idx = i * 3 + j; //顶点的开始
				GMint idx_next = i * 3 + (j + 1) % 3;
				GMint idx_prev = i * 3 + (j + 2) % 3;
				linear_math::Vector2 uv(v[indices[idx] * 3], v[indices[idx] * 3 + 1]);
				linear_math::Vector3 vertex(t[indices[idx] * 3], t[indices[idx] * 3 + 1], t[indices[idx] * 3 + 2]);
				linear_math::Vector3 vertex_prev(t[indices[idx_prev] * 3], t[indices[idx_prev] * 3 + 1], t[indices[idx_prev] * 3 + 2]),
					vertex_next(t[indices[idx_next] * 3], t[indices[idx_next] * 3 + 1], t[indices[idx_next] * 3 + 2]);
				linear_math::Vector3 normal = linear_math::cross(vertex - vertex_prev, vertex_next - vertex);
				normal = linear_math::normalize(normal);

				component->vertex(vertex[0], vertex[1], vertex[2]);
				component->normal(normal[0], normal[1], normal[2]);
				component->uv((uv[0] + 1) / 2, (uv[1] + 1) / 2);
				component->color(1.f, 1.f, 1.f);
			}
			component->endFace();
		}
		body->appendComponent(component);
		coreObj->append(body);
	}

	*obj = coreObj;
}
