#include "stdafx.h"
#include "gmprimitivecreator.h"

void GMPrimitiveCreator::createCube(GMfloat extents[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback, GMMeshType type)
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
		if (shaderCallback)
			shaderCallback->onCreateShader(component->getShader());
		body->appendComponent(component);
		coreObj->append(body);
	}

	*obj = coreObj;
}

void GMPrimitiveCreator::createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback, GMMeshType type, GMCreateAnchor anchor, GMfloat (*customUV)[12])
{
	static constexpr GMfloat v_anchor_center[] = {
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
	};

	const GMfloat(*_customUV)[12] = customUV ? customUV : &v_anchor_center;
	const GMfloat(&uvArr)[12] = *_customUV;

	static constexpr GMfloat v_anchor_top_left[] = {
		0, 0, 0,
		0, -2, 0,
		2, -2, 0,
		2, 0, 0,
	};

	const GMfloat(&v)[12] = (anchor == TopLeft) ? v_anchor_top_left : v_anchor_center;

	static constexpr GMint indices[] = {
		0, 1, 3,
		2, 3, 1,
	};

	GMModel* coreObj = new GMModel();

	// 实体
	GMfloat t[12];
	for (GMint i = 0; i < 12; i++)
	{
		t[i] = extents[i % 3] * v[i] + position[i % 3];
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
				linear_math::Vector2 uv(uvArr[indices[idx] * 3], uvArr[indices[idx] * 3 + 1]);
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
		if (shaderCallback)
			shaderCallback->onCreateShader(component->getShader());
		body->appendComponent(component);
		coreObj->append(body);
	}

	*obj = coreObj;
}

//////////////////////////////////////////////////////////////////////////
void GMPrimitiveUtil::translateModelTo(REF GMModel& model, const GMfloat(&trans)[3])
{
	auto& meshes = model.getAllMeshes();
	GMModelPainter* painter = model.getPainter();
	for (auto& mesh : meshes)
	{
		painter->beginUpdateBuffer(mesh);
		GMfloat* buffer = reinterpret_cast<GMfloat*>(painter->getBuffer());
		GMint vertexCount = mesh->get_transferred_positions_byte_size() / (sizeof(GMfloat) * GMModel::PositionDimension);

		// 计算首个偏移
		GMfloat* first = buffer;
		GMfloat delta[] = {
			trans[0] - first[0],
			trans[1] - first[1],
			trans[2] - first[2]
		};

		for (GMint v = 0; v < vertexCount; ++v)
		{
			GMfloat* ptr = buffer + v * GMModel::PositionDimension;
			ptr[0] += delta[0];
			ptr[1] += delta[1];
			ptr[2] += delta[2];
		}
		painter->endUpdateBuffer();
	}
}

void GMPrimitiveUtil::scaleModel(REF GMModel& model, const GMfloat (&scaling)[3])
{
	auto& meshes = model.getAllMeshes();
	GMModelPainter* painter = model.getPainter();
	for (auto& mesh : meshes)
	{
		painter->beginUpdateBuffer(mesh);
		GMfloat* buffer = reinterpret_cast<GMfloat*>(painter->getBuffer());
		GMint vertexCount = mesh->get_transferred_positions_byte_size() / (sizeof(GMfloat) * GMModel::PositionDimension);
		for (GMint v = 0; v < vertexCount; ++v)
		{
			GMfloat* ptr = buffer + v * GMModel::PositionDimension;
			ptr[0] *= scaling[0];
			ptr[1] *= scaling[1];
			ptr[2] *= scaling[2];
		}
		painter->endUpdateBuffer();
	}
}