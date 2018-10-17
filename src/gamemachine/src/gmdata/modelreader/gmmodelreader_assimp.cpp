#include "stdafx.h"
#include "gmmodelreader_assimp.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/IOSystem.hpp>
#include <../code/MemoryIOWrapper.h>
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
		Assimp::IOStream* s = Assimp::MemoryIOSystem::Open(pFile, pMode);
		if (!s)
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
		return s;
	}

private:
	GMModelLoadSettings m_settings;
};

namespace
{
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

	void processMesh(GMModelReader_Assimp* imp, aiMesh* mesh, const aiScene* scene, GMModel* model)
	{
		model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
		if (mesh->mNumFaces > 0)
			model->setDrawMode(GMModelDrawMode::Index);
		else
			model->setDrawMode(GMModelDrawMode::Vertex);

		GMMesh* m = new GMMesh(model);
		// vertices
		for (auto i = 0u; i < mesh->mNumVertices; ++i)
		{
			Array<GMfloat, 4> colors = mesh->mColors[0] ? 
				Array<GMfloat, 4>{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a } :
				Array<GMfloat, 4>{0};
			Array<GMfloat, 2> tex = mesh->mTextureCoords[0] ?
				Array<GMfloat, 2>{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y } :
				Array<GMfloat, 2>{ 0 };

			GMVertex v = {
				{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },
				{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z },
				{ tex[0], tex[1] },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0 },
				{ colors[0], colors[1], colors[2], colors[3] }
			};
			m->vertex(v);
		}

		// faces
		for (auto i = 0u; i < mesh->mNumFaces; ++i)
		{
			for (auto j = 0u; j < mesh->mFaces[i].mNumIndices; ++j)
			{
				m->index(mesh->mFaces[i].mIndices[j]);
			}
		}

		// materials
		if (scene->mMaterials)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			processMaterial(imp, material, model);
		}

		if (scene->mTextures)
		{
			aiTexture* texture = scene->mTextures[mesh->mMaterialIndex];
			processTexture(imp, texture, model);
		}
	}

	void processNode(GMModelReader_Assimp* imp, aiNode* node, const aiScene* scene, GMModels* models)
	{
		for (auto i = 0u; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			GMModel* model = new GMModel();
			processMesh(imp, mesh, scene, model);
			models->push_back(GMAsset(GMAssetType::Model, model));
		}
		// 接下来对它的子节点重复这一过程
		for (auto i = 0u; i < node->mNumChildren; i++)
		{
			::processNode(imp, node->mChildren[i], scene, models);
		}
	}
}

bool GMModelReader_Assimp::load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset)
{
	D(d);
	getTextureMap().clear();
	d->settings = settings;

	Assimp::Importer imp;
	GMuint32 flag = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices;
	if (settings.flipTexcoords)
		flag |= aiProcess_FlipUVs;

	const std::string fileName = GMPath::filename(settings.filename).toStdString();
	const aiScene* scene = imp.ReadFileFromMemory(
		buffer.buffer, 
		buffer.size, 
		flag,
		new GamePackageIOSystem(settings, (const uint8_t*)buffer.buffer, buffer.size),
		fileName.c_str()
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		gm_error(gm_dbg_wrap("read model error. Reason: {0}"), imp.GetErrorString());
		return false;
	}

	GMModels* models = new GMModels();
	processNode(this, scene->mRootNode, scene, models);
	asset = GMAsset(GMAssetType::Models, models);

	return true;
}

bool GMModelReader_Assimp::test(const GMBuffer& buffer)
{
	return true;
}

