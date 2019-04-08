#include "stdafx.h"
#include "gmwavegameobject.h"
#include <gmutilities.h>
#include <gmgl/shader_constants.h>

#define getVertex(x, y) (vertices[(x) + (y) * (sliceM + 1)])
#define getVertexIndex(x, y) ((x) + (y) * (sliceM + 1))
#define getAdjVert(x, y, pos) getAdjacentVertex(vertices, x, y, sliceM, pos)

#define TO_VEC3(i) GMVec3((i)[0], (i)[1], (i)[2])
#define TO_VEC2(i) GMVec2((i)[0], (i)[1])

struct GMWaveDescriptionStrings
{
	GMString steepness;
	GMString amplitude;
	GMString direction;
	GMString speed;
	GMString waveLength;
};

namespace
{
	GMRenderTechniqueID s_techId;

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

	void calculateTangent(GMVertex& p0, const GMVertex& p1, const GMVertex& p2)
	{
		GMVertex& currentVertex = p0;
		GMVec3 e0, e1, e2;
		GMVec2 uv0, uv1, uv2;

		e0 = TO_VEC3(p0.positions);
		e1 = TO_VEC3(p1.positions);
		e2 = TO_VEC3(p2.positions);
		uv0 = TO_VEC2(p0.texcoords);
		uv1 = TO_VEC2(p1.texcoords);
		uv2 = TO_VEC2(p2.texcoords);

		GMVec3 E1 = e1 - e0;
		GMVec3 E2 = e2 - e0;
		GMVec2 deltaUV1 = uv1 - uv0;
		GMVec2 deltaUV2 = uv2 - uv0;
		const GMfloat& t1 = deltaUV1.getX();
		const GMfloat& b1 = deltaUV1.getY();
		const GMfloat& t2 = deltaUV2.getX();
		const GMfloat& b2 = deltaUV2.getY();

		GMFloat4 f4_E1, f4_E2;
		E1.loadFloat4(f4_E1);
		E2.loadFloat4(f4_E2);

		GMfloat s = 1.0f / (t1*b2 - b1*t2);

		GMfloat tangents[3] = {
			s * (b2 * f4_E1[0] - b1 * f4_E2[0]),
			s * (b2 * f4_E1[1] - b1 * f4_E2[1]),
			s * (b2 * f4_E1[2] - b1 * f4_E2[2])
		};
		GMfloat bitangents[3] = {
			s * (-t2 * f4_E1[0] + t1 * f4_E2[0]),
			s * (-t2 * f4_E1[1] + t1 * f4_E2[1]),
			s * (-t2 * f4_E1[2] + t1 * f4_E2[2])
		};

		GMVec3 tangentVector = FastNormalize(GMVec3(tangents[0], tangents[1], tangents[2]));
		GMVec3 bitangentVector = FastNormalize(GMVec3(bitangents[0], bitangents[1], bitangents[2]));
		GMFloat4 f4_tangentVector, f4_bitangentVector;
		tangentVector.loadFloat4(f4_tangentVector);
		bitangentVector.loadFloat4(f4_bitangentVector);

		currentVertex.tangents[0] = f4_tangentVector[0];
		currentVertex.tangents[1] = f4_tangentVector[1];
		currentVertex.tangents[2] = f4_tangentVector[2];

		currentVertex.bitangents[0] = f4_bitangentVector[0];
		currentVertex.bitangents[1] = f4_bitangentVector[1];
		currentVertex.bitangents[2] = f4_bitangentVector[2];
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

	void calculateNormals2(GMVertices& vertices, const Vector<GMWaveDescription>& ds, GMfloat duration)
	{
		for (GMVertex& v : vertices)
		{
			GMfloat x = 0, y = 0, z = 0;
			for (GMint32 i = 0; i < ds.size(); ++i)
			{
				GMfloat fi = 2 * ds[i].speed / ds[i].waveLength;
				GMfloat wa = ds[i].waveLength * ds[i].amplitude;
				GMfloat arg = ds[i].waveLength * Dot(ds[i].direction, GMVec3(v.positions[0], v.positions[1], v.positions[2])) + fi * duration;
				GMfloat c = Cos(arg);
				GMfloat s = Sin(arg);
				x += ds[i].direction.getX() * v.positions[0] * wa * c;
				y += ds[i].steepness * wa * s;
				z += ds[i].direction.getZ() * v.positions[2] * wa * c;
			}

			GMVec3 n = Normalize(GMVec3(-x, 1 - y, -z));
			v.normals = { n.getX(), n.getY(), n.getZ() };
		}
	}

	void calculateTangentSpace(GMVertices& vertices, GMint32 sliceM, GMint32 sliceN)
	{
		for (GMsize_t i = 0; i < sliceN + 1; ++i)
		{
			for (GMsize_t j = 0; j < sliceM + 1; ++j)
			{
				if (i == 0 && j == 0) //左下角
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				}
				else if (i == 0 && j == sliceM) //右下角
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				}
				else if (i == sliceN && j == 0)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				}
				else if (i == sliceN && j == sliceM)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Down), getAdjVert(j, i, Left));
				}
				// 四条边的情况
				else if (i == 0)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				}
				else if (j == 0)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Up), getAdjVert(j, i, Right));
				}
				else if (j == sliceM)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
				}
				else if (i == sliceN)
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Right), getAdjVert(j, i, Down));
				}
				// 中央
				else
				{
					calculateTangent(getVertex(j, i), getAdjVert(j, i, Left), getAdjVert(j, i, Up));
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
				part->index(getVertexIndex(j, i));
				part->index(getVertexIndex(j, i + 1));
				part->index(getVertexIndex(j + 1, i + 1));

				part->index(getVertexIndex(j, i));
				part->index(getVertexIndex(j + 1, i + 1));
				part->index(getVertexIndex(j + 1, i));
			}
		}

		scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
	}

	GMfloat gerstner_x(const GMWaveDescription& desc, GMVec3 pos, GMfloat targ)
	{
		if (FuzzyCompare(desc.steepness, 0))
			return 0;

		return desc.steepness * desc.amplitude * Dot(desc.direction, GMVec3(pos.getX(), 0, 0)) * Cos(targ);
	}

	GMfloat gerstner_y(const GMWaveDescription& desc, GMVec3 pos, GMfloat targ)
	{
		return desc.amplitude * Sin(targ);
	}

	GMfloat gerstner_z(const GMWaveDescription& desc, GMVec3 pos, GMfloat targ)
	{
		if (FuzzyCompare(desc.steepness, 0))
			return 0;

		return desc.steepness * desc.amplitude * Dot(desc.direction, GMVec3(0, 0, pos.getZ())) * Cos(targ);
	}
}

void GMWaveGameObject::initShader(const IRenderContext* context)
{
	IGraphicEngine* engine = context->getEngine();
	GMRenderTechniques techs;
	GMRenderTechnique vertexTech(GMShaderType::Vertex);
	vertexTech.setCode(
		GMRenderEnvironment::OpenGL,
		L"#version 330\n"
		L"#include \"foundation/foundation.h\"\n"
		L"#include \"foundation/vert_header.h\"\n"
		L"out vec4 _model3d_position_world;"
		L"out vec3 _cubemap_uv;"
		L""
		L"struct GMExt_Wave_WaveDescription"
		L"{"
		L"    float Steepness;"
		L"    float Amplitude;"
		L"    vec3 Direction;"
		L"    float Speed;"
		L"    float WaveLength;"
		L"};"
		L""
		L"const int GM_MaxWaves = 10;"
		L"uniform GMExt_Wave_WaveDescription GM_Ext_Wave_WaveDescriptions[GM_MaxWaves];"
		L"uniform int GM_Ext_Wave_IsWavePlaying = 0;"
		L"uniform int GM_Ext_Wave_WaveCount = 1;"
		L"uniform float GM_Ext_Wave_Duration = 0;"
		L"\n"
		L"float GM_Ext_Wave_gerstner_x(GMExt_Wave_WaveDescription desc, vec3 pos, float targ)\n"
		L"{"
		L"    return desc.Steepness * desc.Amplitude * dot(desc.Direction, vec3(pos.x, 0, 0)) * cos(targ);"
		L"}"
		L"\n"
		L"float GM_Ext_Wave_gerstner_y(GMExt_Wave_WaveDescription desc, vec3 pos, float targ)\n"
		L"{"
		L"    return desc.Amplitude * sin(targ);"
		L"}"
		L"\n"
		L"float GM_Ext_Wave_gerstner_z(GMExt_Wave_WaveDescription desc, vec3 pos, float targ)\n"
		L"{"
		L"    return desc.Steepness * desc.Amplitude * dot(desc.Direction, vec3(0, 0, pos.z)) * cos(targ);"
		L"}"
		L"\n"
		L"void main()"
		L"{"
		L"    init_layouts();"
		L"    vec4 p = position;"
		L"    if (GM_Ext_Wave_IsWavePlaying != 0)"
		L"    {"
		L"        float gerstner_x_sum = 0;"
		L"        float gerstner_y_sum = 0;"
		L"        float gerstner_z_sum = 0;"
		L"        for (int i = 0; i < GM_Ext_Wave_WaveCount; ++i)"
		L"        {"
		L"            float fi = 2 * GM_Ext_Wave_WaveDescriptions[i].Speed / GM_Ext_Wave_WaveDescriptions[i].WaveLength;"
		L"            float targ = GM_Ext_Wave_WaveDescriptions[i].WaveLength * (GM_Ext_Wave_WaveDescriptions[i].Direction.x * p.x + GM_Ext_Wave_WaveDescriptions[i].Direction.z * p.z) + fi * GM_Ext_Wave_Duration;"
		L"            gerstner_x_sum += GM_Ext_Wave_gerstner_x(GM_Ext_Wave_WaveDescriptions[i], p.xyz, targ);"
		L"            gerstner_y_sum += GM_Ext_Wave_gerstner_y(GM_Ext_Wave_WaveDescriptions[i], p.xyz, targ);"
		L"            gerstner_z_sum += GM_Ext_Wave_gerstner_z(GM_Ext_Wave_WaveDescriptions[i], p.xyz, targ);"
		L"        }"
		L""
		L"        p = vec4(p.x + gerstner_x_sum, gerstner_y_sum, p.z + gerstner_z_sum, 1);"
		L"    }"
		L""
		L"    _model3d_position_world = GM_WorldMatrix * p;"
		L"    gl_Position = (GM_ProjectionMatrix * GM_ViewMatrix * _model3d_position_world);"
		L"}"
	);
	techs.addRenderTechnique(vertexTech);
	s_techId = engine->getRenderTechniqueManager()->addRenderTechniques(techs);
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
	waveModel->setTechniqueId(s_techId);
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

void GMWaveGameObject::stop()
{
	D(d);
	if (d->isPlaying)
	{
		d->isPlaying = false;
	}
}

void GMWaveGameObject::update(GMDuration dt)
{
	D(d);
	D_BASE(db, Base);
	d->duration += dt;

	// 使用CPU进行运算，需要手动update每个顶点
	if (!d->waveModel)
		d->waveModel = db->asset.getScene()->getModels()[0].getModel();

	if (d->acceleration == GMWaveGameObjectHardwareAcceleration::CPU)
	{
		updateEachVertex();
		d->waveModel->setType(GMModelType::Model3D);
	}
	else
	{
		d->waveModel->setType(GMModelType::Custom);
	}
}

void GMWaveGameObject::onRenderShader(GMModel* model, IShaderProgram* shaderProgram) const
{
	D(d);
	if (d->acceleration == GMWaveGameObjectHardwareAcceleration::GPU)
	{
		static Vector<GMWaveDescriptionStrings> s_waveDescriptionStrings;
		static const GMString s_isPlaying = L"GM_Ext_Wave_IsWavePlaying";
		static const GMString s_waveCount = L"GM_Ext_Wave_WaveCount";
		static const GMString s_duration = L"GM_Ext_Wave_Duration";
		static std::once_flag s_flag;
		constexpr GMint32 MAX_WAVES = 10;

		std::call_once(s_flag, [MAX_WAVES](Vector<GMWaveDescriptionStrings>& strings) {
			strings.resize(MAX_WAVES);
			for (GMint32 i = 0; i < MAX_WAVES; ++i)
			{
				GMString strIdx = GMString(i);
				strings[i].steepness = L"GM_Ext_Wave_WaveDescriptions[" + strIdx + L"].Steepness";
				strings[i].amplitude = L"GM_Ext_Wave_WaveDescriptions[" + strIdx + L"].Amplitude";
				strings[i].direction = L"GM_Ext_Wave_WaveDescriptions[" + strIdx + L"].Direction";
				strings[i].speed = L"GM_Ext_Wave_WaveDescriptions[" + strIdx + L"].Speed";
				strings[i].waveLength = L"GM_Ext_Wave_WaveDescriptions[" + strIdx + L"].WaveLength";
			}
		}, s_waveDescriptionStrings);

		// 传递参数
		GMsize_t prog = verifyIndicesContainer(d->globalIndices, shaderProgram);
		verifyIndicesContainer(d->waveIndices, shaderProgram);
		GMint32 waveCount = gm_sizet_to_int(d->waveDescriptions.size());
		if (d->waveIndices[prog].size() <= waveCount)
			d->waveIndices[prog].resize(waveCount + 1);

		shaderProgram->setInt(getVariableIndex(shaderProgram, d->globalIndices[prog].isPlaying, s_isPlaying), d->isPlaying ? 1 : 0);
		shaderProgram->setInt(getVariableIndex(shaderProgram, d->globalIndices[prog].waveCount, s_waveCount), waveCount);
		shaderProgram->setFloat(getVariableIndex(shaderProgram, d->globalIndices[prog].duration, s_duration), d->duration);
		for (GMint32 i = 0; i < waveCount; ++i)
		{
			shaderProgram->setFloat(
				getVariableIndex(shaderProgram, d->waveIndices[prog][i].steepness, s_waveDescriptionStrings[i].steepness),
				d->waveDescriptions[i].steepness);

			shaderProgram->setFloat(
				getVariableIndex(shaderProgram, d->waveIndices[prog][i].amplitude, s_waveDescriptionStrings[i].amplitude),
				d->waveDescriptions[i].amplitude);

			shaderProgram->setVec3(
				getVariableIndex(shaderProgram, d->waveIndices[prog][i].direction, s_waveDescriptionStrings[i].direction),
				ValuePointer(d->waveDescriptions[i].direction));

			shaderProgram->setFloat(
				getVariableIndex(shaderProgram, d->waveIndices[prog][i].speed, s_waveDescriptionStrings[i].speed),
				d->waveDescriptions[i].speed);

			shaderProgram->setFloat(
				getVariableIndex(shaderProgram, d->waveIndices[prog][i].waveLength, s_waveDescriptionStrings[i].waveLength),
				d->waveDescriptions[i].waveLength);
		}
	}
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
				GMfloat triarg = d->waveDescriptions[i].waveLength * (d->waveDescriptions[i].direction.getX() * pos.getX() + d->waveDescriptions[i].direction.getZ() * pos.getZ()) + fi * d->duration;
				gerstner_x_sum += gerstner_x(d->waveDescriptions[i], pos, triarg);
				gerstner_y_sum += gerstner_y(d->waveDescriptions[i], pos, triarg);
				gerstner_z_sum += gerstner_z(d->waveDescriptions[i], pos, triarg);
			}

			vertex.positions = {
				vertex.positions[0] + gerstner_x_sum,
				gerstner_y_sum,
				vertex.positions[2] + gerstner_z_sum
			};
		}

		calculateNormals(vertices, getObjectDescription().sliceM, getObjectDescription().sliceN);
		//calculateNormals2(vertices, d->waveDescriptions, d->duration);
		//calculateTangentSpace(vertices, getObjectDescription().sliceM, getObjectDescription().sliceN);

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
