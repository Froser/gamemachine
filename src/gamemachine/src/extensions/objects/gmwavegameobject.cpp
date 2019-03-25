﻿#include "stdafx.h"
#include "gmwavegameobject.h"
#include <gmutilities.h>

#define getVertex(x, y) (vertices[(x) + (y) * (sliceM + 1)])
#define getIndex(x, y) ((x) + (y) * (sliceM + 1))
#define getAdjVert(x, y, pos) getAdjacentVertex(vertices, x, y, sliceM, pos)

namespace
{
	// 按照以下顺序求法线:
	// p0   p1 | p2   p0 |      p2 | p1
	//         |         |         |
	// p2      |      p1 | p1   p0 | p0   p2
	// 法线 = (p1 - p0) * (p2 - p0)
	GMVec3 calculateNormal(const GMVertex& p0, const GMVertex& p1, const GMVertex& p2)
	{
		GMVec3 v0 = GMVec3(p0.positions[0], p0.positions[1], p0.positions[2]);
		GMVec3 v1 = GMVec3(p1.positions[0], p1.positions[1], p1.positions[2]) - v0;
		GMVec3 v2 = GMVec3(p2.positions[0], p2.positions[1], p2.positions[2]) - v0;
		return Normalize(Cross(v1, v2));
	}

	enum VertexPosition
	{
		Left,
		Right,
		Up,
		Down,
	};

	GMVertex& getAdjacentVertex(Vector<GMVertex>& vertices, GMsize_t x, GMsize_t y, GMsize_t sliceM, VertexPosition pos)
	{
		static GMVertex s_invalid;
		switch (pos)
		{
		case Left:
			return getVertex(x - 1, y);
		case Right:
			return getVertex(x + 1, y);
		case Up:
			return getVertex(x, y + 1);
		case Down:
			return getVertex(x, y - 1);
		default:
			GM_ASSERT(false);
			return s_invalid;
		}
	}

	void calculateNormals(GMVertices& vertices, GMint32 sliceM, GMint32 sliceN)
	{
		GM_ASSERT(vertices.size() == (sliceM + 1) * (sliceN + 1));
		for (GMsize_t i = 0; i < sliceN + 1; ++i)
		{
			for (GMsize_t j = 0; j < sliceM + 1; ++j)
			{
				// 角上的顶点，直接计算Normal，边上的顶点取2个三角形Normal平均值，中间的则取4个
				// 四角的情况
				if (i == 0 && j == 0) //左下角
				{
					GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
					getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (i == 0 && j == sliceM) //右下角
				{
					GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
					getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (i == sliceN && j == 0)
				{
					GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
					getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (i == sliceN && j == sliceM)
				{
					GMVec3 normal = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
					getVertex(0, 0).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				// 四条边的情况
				else if (i == 0)
				{
					GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
					GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
					GMVec3 normal = Normalize((normal0 + normal1) / 2);
					getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (j == 0)
				{
					GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
					GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
					GMVec3 normal = Normalize((normal0 + normal1) / 2);
					getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (j == sliceM)
				{
					GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
					GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
					GMVec3 normal = Normalize((normal0 + normal1) / 2);
					getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				else if (i == sliceN)
				{
					GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
					GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
					GMVec3 normal = Normalize((normal0 + normal1) / 2);
					getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
				// 中央
				else
				{
					GMVec3 normal0 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
					GMVec3 normal1 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
					GMVec3 normal2 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
					GMVec3 normal3 = calculateNormal(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
					GMVec3 normal = Normalize((normal0 + normal1 + normal2 + normal3) / 4);
					getVertex(j, i).normals = { normal.getX(), normal.getY(), normal.getZ() };
				}
			}
		}
	}

	void createTerrain(
		const GMWaveGameObjectDescription& desc,
		REF GMSceneAsset& scene
	)
	{
		GMfloat x = desc.terrainX;
		GMfloat z = desc.terrainZ;
		const GMfloat x_start = x;
		const GMfloat z_start = z;
		const GMfloat& sliceM = desc.sliceM;
		const GMfloat& sliceN = desc.sliceN;

		GMVertices vertices;
		vertices.reserve((sliceM + 1) * (sliceN + 1));

		const GMfloat dx = desc.terrainLength / sliceM; // 2D横向
		const GMfloat dz = desc.terrainWidth / sliceN; // 2D纵向

		// 先计算顶点坐标
		GMfloat y = 0;
		GMfloat u = 0, v = 0;

		for (GMsize_t i = 0; i < sliceN + 1; ++i)
		{
			for (GMsize_t j = 0; j < sliceM + 1; ++j)
			{
				GMfloat x_distance = x - x_start;
				y = 0;

				u = (x_distance) / desc.textureLength;
				v = (z - z_start) / desc.textureHeight;

				GMVertex vert = { { x, y, z },{ 0, 0, 0 },{ u, v } };
				vertices.push_back(std::move(vert));
				x += dx;
			}

			z += dz;
			x = x_start;
		}

		// 再计算法线，一个顶点的法线等于相邻三角形平均值
		calculateNormals(vertices, sliceM, sliceN);

		// 顶点数据创建完毕
		GMModel* m = new GMModel();
		GMPart* part = new GMPart(m);
		part->swap(vertices);

		// 接下来创建indices
		m->setUsageHint(GMUsageHint::DynamicDraw);
		m->setDrawMode(GMModelDrawMode::Index);
		m->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
		for (GMsize_t i = 0; i < sliceN; ++i)
		{
			for (GMsize_t j = 0; j < sliceM; ++j)
			{
				part->index(getIndex(j, i));
				part->index(getIndex(j, i + 1));
				part->index(getIndex(j + 1, i + 1));

				part->index(getIndex(j, i));
				part->index(getIndex(j + 1, i + 1));
				part->index(getIndex(j + 1, i));
			}
		}

		scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
	}

	GMfloat gerstner_x(const GMWaveDescription& desc, GMfloat fi, GMVec3 pos, GMfloat t)
	{
		if (FuzzyCompare(desc.steepness, 0))
			return 0;

		return desc.steepness * desc.amplitude * Dot(desc.direction, GMVec3(pos.getX(), 0, 0)) * Cos(desc.waveLength * Dot(desc.direction, pos) + fi * t);
	}

	GMfloat gerstner_y(const GMWaveDescription& desc, GMfloat fi, GMVec3 pos, GMfloat t)
	{
		return desc.amplitude * Sin(desc.waveLength * Dot(desc.direction, pos) + fi * t);
	}

	GMfloat gerstner_z(const GMWaveDescription& desc, GMfloat fi, GMVec3 pos, GMfloat t)
	{
		if (FuzzyCompare(desc.steepness, 0))
			return 0;

		return desc.steepness * desc.amplitude * Dot(desc.direction, GMVec3(0, 0, pos.getZ())) * Cos(desc.waveLength * Dot(desc.direction, pos) + fi * t);
	}
}

GMWaveGameObject* GMWaveGameObject::create(const GMWaveGameObjectDescription& desc)
{
	GMWaveGameObject* ret = new GMWaveGameObject();
	GMSceneAsset waveScene;
	createTerrain(desc, waveScene);
	ret->setAsset(waveScene);
	ret->setObjectDescription(desc);

	GMModel* waveModel = waveScene.getScene()->getModels()[0].getModel();
	ret->setVertices(waveModel->getParts()[0]->vertices());
	return ret;
}

void GMWaveGameObject::setWaveDescriptions(Vector<GMWaveDescription> desc)
{
	D(d);
	d->waveDescriptions = std::move(desc);
}

void GMWaveGameObject::play()
{
	D(d);
	if (!d->isPlaying)
	{
		d->isPlaying = true;
	}
}

void GMWaveGameObject::update(GMDuration dt)
{
	D(d);
	d->duration += dt;
	updateEachVertex();
}

void GMWaveGameObject::updateEachVertex()
{
	D(d);
	D_BASE(db, Base);
	if (d->isPlaying)
	{
		GMVertices vertices = d->vertices;
		for (GMVertex& vertex : vertices)
		{
			GMfloat gerstner_x_sum = 0;
			GMfloat gerstner_y_sum = 0;
			GMfloat gerstner_z_sum = 0;
			GMVec3 pos = { vertex.positions[0], vertex.positions[1], vertex.positions[2] };
			for (GMsize_t i = 0; i < d->waveDescriptions.size(); ++i)
			{
				GMfloat fi = 2 * d->waveDescriptions[i].speed / d->waveDescriptions[i].waveLength;
				gerstner_x_sum += gerstner_x(d->waveDescriptions[i], fi, pos, d->duration);
				gerstner_y_sum += gerstner_y(d->waveDescriptions[i], fi, pos, d->duration);
				gerstner_z_sum += gerstner_z(d->waveDescriptions[i], fi, pos, d->duration);
			}

			vertex.positions = {
				vertex.positions[0] + gerstner_x_sum,
				gerstner_y_sum,
				vertex.positions[2] + gerstner_z_sum
			};
		}
		calculateNormals(vertices, getObjectDescription().sliceM, getObjectDescription().sliceN);
		GMModelDataProxy* proxy = getModel()->getModelDataProxy();
		proxy->beginUpdateBuffer(GMModelBufferType::VertexBuffer);
		void* ptr = proxy->getBuffer();
		GMsize_t sz = sizeof(GMVertex) * vertices.size();
		memcpy_s(ptr, sz, vertices.data(), sz);
		proxy->endUpdateBuffer();
	}
}

void GMWaveGameObject::setVertices(const GMVertices& vertices)
{
	D(d);
	d->vertices = vertices;
}
