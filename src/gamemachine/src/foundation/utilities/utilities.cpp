#include "stdafx.h"
#include "utilities.h"
#include "foundation/gamemachine.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/gmimagebuffer.h"

#define getVertex(x, y) (vertices[(x) + (y) * (sliceM + 1)])
#define getIndex(x, y) ((x) + (y) * (sliceM + 1))

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
}

const GMVec3& GMPrimitiveCreator::one3()
{
	static GMVec3 o (1, 1, 1);
	return o;
}

const GMVec2& GMPrimitiveCreator::one2()
{
	static GMVec2 o(1, 1);
	return o;
}

GMfloat* GMPrimitiveCreator::origin()
{
	static GMfloat o[3] = { 0 };
	return o;
}

void GMPrimitiveCreator::createCube(const GMVec3& halfExtents, REF GMSceneAsset& scene)
{
	static const GMVec3 s_vertices[8] = {
		GMVec3(-1, -1, -1),
		GMVec3(-1, 1, -1),
		GMVec3(1, -1, -1),
		GMVec3(1, 1, -1),

		GMVec3(-1, -1, 1),
		GMVec3(-1, 1, 1),
		GMVec3(1, -1, 1),
		GMVec3(1, 1, 1),
	};

	GMVec3 vertices_vec[8];
	GMFloat4 vertices[8];
	for (GMint32 i = 0; i < GM_array_size(vertices); ++i)
	{
		vertices_vec[i] = s_vertices[i] * halfExtents;
		vertices_vec[i].loadFloat4(vertices[i]);
	}

	/*
	采用
	1 3
	0 2
	顶点顺序来绘制每一个面
	*/

	GMModel* m = new GMModel();
	m->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	m->setType(GMModelType::Model3D);
	GMPart* face = new GMPart(m);

	//Front
	{
		GMVertex V0 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 0, 0, -1.f }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Back
	{
		GMVertex V0 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ 0, 0, 1.f }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Left
	{
		GMVertex V0 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ -1.f, 0, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Right
	{
		GMVertex V0 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 1.f, 0, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Bottom
	{
		GMVertex V0 = {
			{ vertices[4][0], vertices[4][1], vertices[4][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[6][0], vertices[6][1], vertices[6][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
			{ 0, -1.f, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	//Top
	{
		GMVertex V0 = {
			{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 0, 1 }, //texcoord
		},
		V1 = {
			{ vertices[5][0], vertices[5][1], vertices[5][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 0, 0 }, //texcoord
		},
		V2 = {
			{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 1, 1 }, //texcoord
		},
		V3 = {
			{ vertices[7][0], vertices[7][1], vertices[7][2] }, //position
			{ 0, 1.f, 0 }, //normal
			{ 1, 0 }, //texcoord
		};
		face->vertex(V0);
		face->vertex(V1);
		face->vertex(V2);
		face->vertex(V1);
		face->vertex(V3);
		face->vertex(V2);
	}

	scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
}

void GMPrimitiveCreator::createQuadrangle(const GMVec2& halfExtents, GMfloat z, REF GMSceneAsset& scene)
{
	constexpr GMfloat texcoord[4][2] =
	{
		{ 0, 1 },
		{ 0, 0 },
		{ 1, 1 },
		{ 1, 0 },
	};

	// 排列方式：
	// 2 4
	// 1 3
	const GMfloat x = halfExtents.getX(), y = halfExtents.getY();
	const GMfloat s_vertices[4][3] = {
		{ -x, -y, z },
		{ -x, y, z },
		{ x, -y, z },
		{ x, y, z },
	};

	GMModel* m = new GMModel();
	m->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	GMPart* part = new GMPart(m);

	GMVertex V1 = {
		{ s_vertices[0][0], s_vertices[0][1], s_vertices[0][2] }, //position
		{ 0, 0, -1.f }, //normal
		{ texcoord[0][0], texcoord[0][1] }, //texcoord
	},
	V2 = {
		{ s_vertices[1][0], s_vertices[1][1], s_vertices[1][2] }, //position
		{ 0, 0, -1.f }, //normal
		{ texcoord[1][0], texcoord[1][1] }, //texcoord
	},
	V3 = {
		{ s_vertices[2][0], s_vertices[2][1], s_vertices[2][2] }, //position
		{ 0, 0, -1.f }, //normal
		{ texcoord[2][0], texcoord[2][1] }, //texcoord
	},
	V4 = {
		{ s_vertices[3][0], s_vertices[3][1], s_vertices[3][2] }, //position
		{ 0, 0, -1.f }, //normal
		{ texcoord[3][0], texcoord[3][1] }, //texcoord
	};
	part->vertex(V1);
	part->vertex(V2);
	part->vertex(V3);
	part->vertex(V4);

	scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
}

void GMPrimitiveCreator::createSphere(GMfloat radius, GMint32 segmentsX, GMint32 segmentsY, REF GMSceneAsset& scene)
{
	GM_ASSERT(radius > 0 && segmentsX > 1 && segmentsY > 1);
	GMModel* m = new GMModel();
	m->setDrawMode(GMModelDrawMode::Index);
	m->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	GMPart* part = new GMPart(m);

	for (GMint32 y = 0; y <= segmentsY; ++y)
	{
		for (GMint32 x = 0; x <= segmentsX; ++x)
		{
			GMfloat xSegment = (GMfloat)x / segmentsX;
			GMfloat ySegment = (GMfloat)y / segmentsY;
			GMfloat xPos = Cos(xSegment * 2.0f * PI) * Sin(ySegment * PI);
			GMfloat yPos = Cos(ySegment * PI);
			GMfloat zPos = -Sin(xSegment * 2.0f * PI) * Sin(ySegment * PI);
			GMVertex v = {
				{ xPos * radius, yPos * radius, zPos * radius },
				{ xPos, yPos, zPos },
				{ xSegment, ySegment }
			};
			part->vertex(v);
		}
	}

	bool oddRow = false;
	for (GMint32 y = 0; y < segmentsY; ++y)
	{
		if (!oddRow)
		{
			for (GMint32 x = 0; x <= segmentsX; ++x)
			{
				part->index(y       * (segmentsX + 1) + x);
				part->index((y + 1) * (segmentsX + 1) + x);
			}
		}
		else
		{
			for (GMint32 x = segmentsX; x >= 0; --x)
			{
				part->index((y + 1) * (segmentsX + 1) + x);
				part->index(y       * (segmentsX + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
}

void GMPrimitiveCreator::createTerrain(
	const GMTerrainDescription& desc,
	REF GMSceneAsset& scene
)
{
	// 从灰度图创建地形
	GMfloat x = desc.terrainX;
	GMfloat z = desc.terrainZ;
	const GMfloat x_start = x;
	const GMfloat z_start = z;
	const GMfloat& sliceM = desc.sliceM;
	const GMfloat& sliceN = desc.sliceN;

	GMVertices vertices;
	vertices.reserve( (sliceM + 1) * (sliceN + 1) );

	const GMfloat dx = desc.terrainLength / sliceM; // 2D横向
	const GMfloat dz = desc.terrainWidth / sliceN; // 2D纵向

	// 先计算顶点坐标
	GMfloat y = 0;
	GMfloat u = 0, v = 0;
	GMint32 x_image = 0, y_image = 0;
	
	for (GMsize_t i = 0; i < sliceN + 1; ++i)
	{
		for (GMsize_t j = 0; j < sliceM + 1; ++j)
		{
			GMfloat x_distance = x - x_start;
			x_image = x_distance * desc.dataWidth / desc.terrainLength;

			if (desc.data)
				y = desc.heightScaling * desc.data[(x_image + y_image * desc.dataWidth) * desc.dataStride] / 0xFF;
			else
				y = 0;

			u = (x_distance) / desc.textureLength;
			v = (z - z_start) / desc.textureHeight;

			GMVertex vert = { { x, y, z }, { 0, 0, 0 }, { u, v } };
			vertices.push_back(std::move(vert));
			x += dx;
		}

		z += dz;
		x = x_start;
		if (desc.data)
			y_image = (z - z_start) * desc.dataHeight / desc.terrainWidth;
	}

	// 再计算法线，一个顶点的法线等于相邻三角形平均值
#define getAdjVert(x, y, pos) getAdjacentVertex(vertices, x, y, sliceM, pos)
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
#undef getAdjVert

	GMModel* m = new GMModel();
	GMPart* part = new GMPart(m);
	part->swap(vertices);

	// 顶点数据创建完毕
	// 接下来创建indices
	if (desc.dynamic)
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

GMTextureAsset GMToolUtil::createTexture(const IRenderContext* context, const GMString& filename, REF GMint32* width, REF GMint32* height)
{
	GMImage* img = nullptr;
	GMBuffer buf;
	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, filename, &buf);
	GMImageReader::load(buf.getData(), buf.getSize(), &img);
	GM_ASSERT(img);

	GMTextureAsset texture;
	GM.getFactory()->createTexture(context, img, texture);
	GM_ASSERT(!texture.isEmpty());

	if (width)
		*width = img->getWidth();

	if (height)
		*height = img->getHeight();

	img->destroy();
	return std::move(texture);
}

void GMToolUtil::createTextureFromFullPath(const IRenderContext* context, const GMString& filename, REF GMTextureAsset& texture, REF GMint32* width, REF GMint32* height)
{
	GMImage* img = nullptr;
	GMBuffer buf;
	GM.getGamePackageManager()->readFileFromPath(filename, &buf);
	GMImageReader::load(buf.getData(), buf.getSize(), &img);
	GM_ASSERT(img);

	GM.getFactory()->createTexture(context, img, texture);
	GM_ASSERT(!texture.isEmpty());

	if (width)
		*width = img->getWidth();

	if (height)
		*height = img->getHeight();

	img->destroy();
}

void GMToolUtil::addTextureToShader(GMShader& shader, GMAsset texture, GMTextureType type)
{
	GM_ASSERT(texture.getType() == GMAssetType::Texture);
	auto& frames = shader.getTextureList().getTextureSampler(type);
	frames.addFrame(texture);
}

bool GMToolUtil::createPBRTextures(
	const IRenderContext* context,
	const GMString& albedoPath,
	const GMString& metallicPath,
	const GMString& roughnessPath,
	const GMString& aoPath,
	const GMString& normalPath,
	REF GMTextureAsset& albedoTexture,
	REF GMTextureAsset& metallicRoughnessAoTexture,
	REF GMTextureAsset& normalTexture
)
{
	bool useWhiteAO = aoPath.isEmpty();

	albedoTexture = GMToolUtil::createTexture(context, albedoPath);
	normalTexture = GMToolUtil::createTexture(context, normalPath);
	if (albedoTexture.isEmpty() || normalPath.isEmpty())
		return false;

	GMBuffer metallicBuf, roughnessBuf, aoBuf;
	GMImage* metallicImg = nullptr, *roughnessImg = nullptr, *aoImg = nullptr;

	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, metallicPath, &metallicBuf);
	if (!GMImageReader::load(metallicBuf.getData(), metallicBuf.getSize(), &metallicImg))
	{
		if (metallicImg)
			metallicImg->destroy();
		return false;
	}

	GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, roughnessPath, &roughnessBuf);
	if (!GMImageReader::load(roughnessBuf.getData(), roughnessBuf.getSize(), &roughnessImg))
	{
		if (metallicImg)
			metallicImg->destroy();
		if (roughnessImg)
			roughnessImg->destroy();
		return false;
	}

	if (!useWhiteAO)
	{
		GM.getGamePackageManager()->readFile(GMPackageIndex::Textures, aoPath, &aoBuf);
		if (!GMImageReader::load(aoBuf.getData(), aoBuf.getSize(), &aoImg))
		{
			if (metallicImg)
				metallicImg->destroy();
			if (roughnessImg)
				roughnessImg->destroy();
			if (aoImg)
				aoImg->destroy();
			return false;
		}
	}

	GMint32 mw = metallicImg->getWidth(), mh = metallicImg->getHeight();
	GMint32 rw = roughnessImg->getWidth(), rh = roughnessImg->getHeight();
	GMint32 aow = aoImg ? aoImg->getWidth() : mw, aoh = aoImg ? aoImg->getHeight() : mh;

	if (mw == rw && rw == aow && mh == rh && rh == aoh)
	{
		GMint32 metallicStep = metallicImg->getData().channels;
		GMint32 roughnessStep = roughnessImg->getData().channels;

		GMImage combinedImage;
		GMImage::Data& data = combinedImage.getData();
		data.target = GMImageTarget::Texture2D;
		data.mipLevels = 1;
		data.format = GMImageFormat::RGBA;
		data.internalFormat = GMImageInternalFormat::RGBA8;
		data.type = GMImageDataType::UnsignedByte;
		data.mip[0].height = metallicImg->getHeight();
		data.mip[0].width = metallicImg->getWidth();

		GMint32 sz = data.mip[0].width * data.mip[0].height * 4;
		data.mip[0].data = new GMbyte[sz];
		GMbyte* metallicPtr = metallicImg->getData().mip[0].data;
		GMbyte* roughnessPtr = roughnessImg->getData().mip[0].data;

		GMint32 aoStep = aoImg ? aoImg->getData().channels : 0;
		GMbyte* aoPtr = aoImg ? aoImg->getData().mip[0].data : nullptr;
		for (GMint32 p = 0; p < sz; p+=4, metallicPtr+=metallicStep, roughnessPtr+=roughnessStep, aoPtr+=aoStep)
		{
			data.mip[0].data[p] = *metallicPtr;
			data.mip[0].data[p + 1] = *roughnessPtr;
			data.mip[0].data[p + 2] = useWhiteAO ? 0xFF : *aoPtr;
			data.mip[0].data[p + 3] = 0xFF;
		}

		GM.getFactory()->createTexture(context, &combinedImage, metallicRoughnessAoTexture);
		if (metallicImg)
			metallicImg->destroy();
		if (roughnessImg)
			roughnessImg->destroy();
		if (aoImg)
			aoImg->destroy();
		return true;
	}
	return false;
}