#include "stdafx.h"
#include "gmmodelreader_assimp.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <../code/MemoryIOWrapper.h>
#include <../code/BaseImporter.h>
#include <../code/StringUtils.h>
#include "foundation/gamemachine.h"
#include "foundation/utilities/utilities.h"

class GamePackageIOSystem : public Assimp::MemoryIOSystem
{
public:
	GamePackageIOSystem(const GMModelLoadSettings& settings, const uint8_t* buff, GMsize_t len)
		: Assimp::MemoryIOSystem(buff, len)
		, m_settings(settings)
	{
	}

	Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override
	{
		if (strncmp(pFile, AI_MEMORYIO_MAGIC_FILENAME, AI_MEMORYIO_MAGIC_FILENAME_LENGTH))
		{
			// 通过GamePackage获取文件
			GMString fn = m_settings.directory;
			if (fn[fn.length() - 1] != '\\' &&
				fn[fn.length() - 1] != '/')
			{
				fn.append("/");
			}
			fn.append(pFile);
			GMBuffer buffer;

			if (m_settings.type == GMModelPathType::Relative)
				GM.getGamePackageManager()->readFile(GMPackageIndex::Models, fn, &buffer);
			else
				GM.getGamePackageManager()->readFileFromPath(fn, &buffer);
			buffer.needRelease = false;
			return new Assimp::MemoryIOStream(buffer.buffer, buffer.size);
		}
		else
		{
			// 带有magic标志，需要进一步检查
			// 首先，去掉magic标识
			const char* fileName = pFile + AI_MEMORYIO_MAGIC_FILENAME_LENGTH;
			// 如果magic之后的文件名和当前文件名不同，则表示尝试读取另外一个文件
			if (fileName[0] == '.')
				++fileName;

			if (GMString(fileName) == GMPath::filename(m_settings.filename))
				return new Assimp::MemoryIOStream(buffer, length);

			return Open(fileName, pMode);
		}
	}

private:
	GMModelLoadSettings m_settings;
};

namespace
{
	// aiMatrix4x4转换函数
	GMMat4 fromAiMatrix(const aiMatrix4x4& mat)
	{
		GMMat4 m;
		GMFloat16 f16 = {
			GMFloat4(mat.a1, mat.b1, mat.c1, mat.d1),
			GMFloat4(mat.a2, mat.b2, mat.c2, mat.d2),
			GMFloat4(mat.a3, mat.b3, mat.c3, mat.d3),
			GMFloat4(mat.a4, mat.b4, mat.c4, mat.d4)
		};
		m.setFloat16(f16);
		return m;
	}

	GMSkeleton* getSkeleton(GMModel* model)
	{
		GMSkeleton* skeleton = nullptr;
		if (!(skeleton = model->getSkeleton()))
		{
			skeleton = new GMSkeleton();
			model->setSkeleton(skeleton);
		}
		return skeleton;
	}

	void materialGet(aiMaterial* material, const char* pKey, GMuint32 type, GMuint32 idx, GMfloat& valueRef)
	{
		GMfloat f;
		aiReturn aiRet = material->Get(pKey, type, idx, f);
		if (aiRet == aiReturn_SUCCESS)
			valueRef = f;
	}

	void materialGet(aiMaterial* material, const char* pKey, GMuint32 type, GMuint32 idx, GMVec3& valueRef)
	{
		aiColor3D v;
		aiReturn aiRet = material->Get(pKey, type, idx, v);
		if (aiRet == aiReturn_SUCCESS)
			valueRef = GMVec3(v.r, v.g, v.b);
	}

	void materialTextureGet(GMModelReader_Assimp* imp, aiMaterial* material, aiTextureType tt, GMModel* model, GMTextureType targetTt)
	{
		GMShader& shader = model->getShader();
		aiString p;
		aiReturn aiRet = material->GetTexture(tt, 0, &p);
		if (aiRet == aiReturn_SUCCESS)
		{
			GMString name = p.C_Str();
			GMTextureAsset tex = imp->getTextureMap()[name];
			if (tex.isEmpty())
			{
				GMString imgPath = GMPath::fullname(GM.getGamePackageManager()->pathOf(GMPackageIndex::Models, imp->getSettings().directory), name);
				GMToolUtil::createTextureFromFullPath(imp->getSettings().context, imgPath, tex);
			}
			if (!tex.isEmpty())
			{
				imp->getTextureMap()[name] = tex;
				GMToolUtil::addTextureToShader(shader, tex, targetTt);
			}
		}
	}

	void processTexture(GMModelReader_Assimp* imp, aiTexture* texture, GMModel* model)
	{
	}

	void processMaterial(GMModelReader_Assimp* imp, aiMaterial* material, GMModel* model)
	{
		GMShader& shader = model->getShader();
		GMMaterial& mtl = shader.getMaterial();
		materialGet(material, AI_MATKEY_COLOR_AMBIENT, mtl.ka);
		materialGet(material, AI_MATKEY_COLOR_DIFFUSE, mtl.kd);
		materialGet(material, AI_MATKEY_COLOR_SPECULAR, mtl.ks);
		materialGet(material, AI_MATKEY_SHININESS, mtl.shininess);
		materialGet(material, AI_MATKEY_COLOR_REFLECTIVE, mtl.refractivity);
		materialTextureGet(imp, material, aiTextureType_AMBIENT, model, GMTextureType::Ambient);
		materialTextureGet(imp, material, aiTextureType_DIFFUSE, model, GMTextureType::Diffuse);
		materialTextureGet(imp, material, aiTextureType_SPECULAR, model, GMTextureType::Specular);
		materialTextureGet(imp, material, aiTextureType_NORMALS, model, GMTextureType::NormalMap);
	}

	void processBones(GMModelReader_Assimp* imp, aiMesh* part, GMModel* model)
	{
		GMSkeleton* skeleton = getSkeleton(model);
		auto& bones = skeleton->getBones();
		auto& boneMapping = bones.getBoneNameIndexMap();
		auto& vertexBoneData = bones.getVertexData();
		vertexBoneData.resize(part->mNumVertices);
		bones.getBones().resize(part->mNumBones);

		GMsize_t index = 0;
		GMsize_t boneIndex = 0;
		for (GMsize_t i = 0; i < part->mNumBones; ++i)
		{
			// 记录骨骼名字和其索引
			GMString boneName = part->mBones[i]->mName.C_Str();
			if (boneMapping.find(boneName) == boneMapping.end())
			{
				// 这是一个新的骨骼
				GMSkeletalBone& bone = bones.getBones()[index];
				bone.name = boneName;
				bone.targetModel = model;
				bone.offsetMatrix = fromAiMatrix(part->mBones[i]->mOffsetMatrix);
				boneIndex = boneMapping[boneName] = index;
				++index;
			}
			else
			{
				boneIndex = index = boneMapping[boneName];
			}

			// 接下来，记录所有的weight
			for (GMsize_t j = 0; j < part->mBones[i]->mNumWeights; ++j)
			{
				auto& weight = part->mBones[i]->mWeights[j];
				GMsize_t vertexId = weight.mVertexId;
				// 将每个顶点与Bones和Weight绑定
				vertexBoneData[vertexId].addData(boneIndex, weight.mWeight);
			}
		}
	}

	GMSkeletalNode* createNodeTree(aiNode* node, GMSkeletalNode* parent)
	{
		GMSkeletalNode* sn = new GMSkeletalNode();
		sn->setName(node->mName.C_Str());
		sn->setParent(parent);
		sn->setTransformToParent(fromAiMatrix(node->mTransformation));

		for (auto i = 0u; i < node->mNumChildren; ++i)
		{
			GMSkeletalNode* childNode = createNodeTree(node->mChildren[i], sn);
			sn->getChildren().push_back(childNode);
		}

		return sn;
	}

	void processAnimation(GMModelReader_Assimp* imp, const aiScene* scene, GMModels* models)
	{
		for (auto modelAsset : models->getModels())
		{
			GM_ASSERT(modelAsset.getModel());
			modelAsset.getModel()->setUsageHint(GMUsageHint::DynamicDraw);
		}

		GMSkeletalAnimations* animations = new GMSkeletalAnimations();
		for (auto a = 0u; a < scene->mNumAnimations; ++a)
		{
			GMSkeletalAnimation ani;
			aiAnimation* animation = scene->mAnimations[a];
			ani.frameRate = animation->mTicksPerSecond;
			ani.duration = animation->mDuration;

			for (auto i = 0u; i < animation->mNumChannels; ++i)
			{
				GMSkeletalAnimationNode n;
				const aiNodeAnim* node = animation->mChannels[i];
				n.name = node->mNodeName.C_Str();
				for (auto j = 0u; j < node->mNumPositionKeys; ++j)
				{
					GMDuration time = node->mPositionKeys[j].mTime;
					const auto& position = node->mPositionKeys[j].mValue;
					n.positions.emplace_back(time, GMVec3(position.x, position.y, position.z));
				}
				for (auto j = 0u; j < node->mNumRotationKeys; ++j)
				{
					GMDuration time = node->mRotationKeys[j].mTime;
					const auto& quat = node->mRotationKeys[j].mValue;
					n.rotations.emplace_back(time, GMQuat(quat.x, quat.y, quat.z, quat.w));
				}
				for (auto j = 0u; j < node->mNumScalingKeys; ++j)
				{
					GMDuration time = node->mScalingKeys[j].mTime;
					const auto& scaling = node->mScalingKeys[j].mValue;
					n.scalings.emplace_back(time, GMVec3(scaling.x, scaling.y, scaling.z));
				}

				ani.nodes.push_back(std::move(n));
			}

			animations->getAnimations().push_back(std::move(ani));
		}

		models->setAnimations(animations);
	}

	void processMesh(GMModelReader_Assimp* imp, aiMesh* part, const aiScene* scene, GMModel* model)
	{
		model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
		if (part->HasFaces())
			model->setDrawMode(GMModelDrawMode::Index);
		else
			model->setDrawMode(GMModelDrawMode::Vertex);

		GMPart* p = new GMPart(model);
		// vertices
		for (auto i = 0u; i < part->mNumVertices; ++i)
		{
			Array<GMfloat, 4> colors = part->mColors[0] ? 
				Array<GMfloat, 4>{ part->mColors[0][i].r, part->mColors[0][i].g, part->mColors[0][i].b, part->mColors[0][i].a } :
				Array<GMfloat, 4>{0};
			Array<GMfloat, 2> tex = part->mTextureCoords[0] ?
				Array<GMfloat, 2>{ part->mTextureCoords[0][i].x, part->mTextureCoords[0][i].y } :
				Array<GMfloat, 2>{ 0 };

			GMVertex v = {
				{ part->mVertices[i].x, part->mVertices[i].y, part->mVertices[i].z },
				{ part->mNormals[i].x, part->mNormals[i].y, part->mNormals[i].z },
				{ tex[0], tex[1] },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0 },
				{ colors[0], colors[1], colors[2], colors[3] }
			};
			p->vertex(v);
		}

		// faces
		if (part->HasFaces())
		{
			for (auto i = 0u; i < part->mNumFaces; ++i)
			{
				for (auto j = 0u; j < part->mFaces[i].mNumIndices; ++j)
				{
					p->index(part->mFaces[i].mIndices[j]);
				}
			}
		}

		// materials
		if (scene->mMaterials)
		{
			aiMaterial* material = scene->mMaterials[part->mMaterialIndex];
			processMaterial(imp, material, model);
		}

		if (scene->mTextures)
		{
			aiTexture* texture = scene->mTextures[part->mMaterialIndex];
			processTexture(imp, texture, model);
		}
	}

	void processMeshes(GMModelReader_Assimp* imp, const aiScene* scene, GMModels* models)
	{
		for (auto i = 0u; i < scene->mNumMeshes; i++)
		{
			aiMesh* part = scene->mMeshes[i];
			GMModel* model = new GMModel();

			// part
			processMesh(imp, part, scene, model);

			// bones
			if (part->HasBones())
				processBones(imp, part, model);

			models->push_back(GMAsset(GMAssetType::Model, model));
		}
	}

	/*
	void processNode(GMModelReader_Assimp* imp, aiNode* node, const aiScene* scene, GMModels* models)
	{
		for (auto i = 0u; i < node->mNumMeshes; i++)
		{
			aiMesh* part = scene->mMeshes[node->mMeshes[i]];
			GMModel* model = new GMModel();

			// part
			processMesh(imp, part, scene, model);

			// bones
			if (part->HasBones())
				processBones(imp, part, model, models);

			models->push_back(GMAsset(GMAssetType::Model, model));
		}

		// 接下来对它的子节点重复这一过程
		for (auto i = 0u; i < node->mNumChildren; i++)
		{
			::processNode(imp, node->mChildren[i], scene, models);
		}
	}
	*/
}

bool GMModelReader_Assimp::load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset)
{
	D(d);
	getTextureMap().clear();
	d->settings = settings;

	Assimp::Importer imp;
	GMuint32 flag = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;

	const std::string fileName = GMPath::filename(d->settings.filename).toStdString();
	const aiScene* scene = imp.ReadFileFromMemory(
		buffer.buffer, 
		buffer.size, 
		flag,
		new GamePackageIOSystem(d->settings, (const uint8_t*)buffer.buffer, buffer.size),
		fileName.c_str()
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		gm_error(gm_dbg_wrap("read model error. Reason: {0}"), imp.GetErrorString());
		return false;
	}

	GMModels* models = new GMModels();
	if (scene->HasAnimations())
	{
		models->setRootNode(createNodeTree(scene->mRootNode, nullptr));
	}

	// nodes
	processMeshes(this, scene, models);

	// animations
	if (scene->HasAnimations())
		processAnimation(this, scene, models);

	asset = GMAsset(GMAssetType::Models, models);

	return true;
}

bool GMModelReader_Assimp::test(const GMModelLoadSettings& settings, const GMBuffer& buffer)
{
	constexpr GMsize_t bufferSize(Assimp::Importer::MaxLenHint + 28);
	const std::string fileName = GMPath::filename(settings.filename).toStdString();

	GamePackageIOSystem ioHandler(settings, buffer.buffer, buffer.size);
	Assimp::Importer imp;
	auto cnt = imp.GetImporterCount();
	for (decltype(cnt) i = 0; i < cnt; ++i)
	{
		Assimp::BaseImporter* bi = imp.GetImporter(i);
		char fbuff[bufferSize];
		ai_snprintf(fbuff, bufferSize, "%s.%s", AI_MEMORYIO_MAGIC_FILENAME, fileName.c_str());
		if (bi->CanRead(fbuff, &ioHandler, false))
			return true;
	}

	for (decltype(cnt) i = 0; i < cnt; ++i)
	{
		Assimp::BaseImporter* bi = imp.GetImporter(i);
		char fbuff[bufferSize];
		ai_snprintf(fbuff, bufferSize, "%s.%s", AI_MEMORYIO_MAGIC_FILENAME, fileName.c_str());
		if (bi->CanRead(fbuff, &ioHandler, true))
			return true;
	}

	return false;
}

