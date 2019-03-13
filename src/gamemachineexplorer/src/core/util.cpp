#include "stdafx.h"
#include "util.h"

namespace core
{
	void utCreatePlain(const GMPlainDescription& desc, GMSceneAsset& scene)
	{
		// 从灰度图创建地形
		GMfloat x = desc.X;
		const GMfloat& y = desc.Y;
		GMfloat z = desc.Z;
		const GMfloat x_start = x;
		const GMfloat x_end = x_start + desc.length;
		const GMfloat z_start = z;
		const GMfloat z_end = z_start + desc.width;

		const GMfloat& sliceM = desc.sliceM;
		const GMfloat& sliceN = desc.sliceN;

		GMVertices vertices;
		vertices.reserve((sliceM + 1) * (sliceN + 1));

		const GMfloat dx = desc.length / sliceM; // 2D横向
		const GMfloat dz = desc.width / sliceN; // 2D纵向

		// 画完横线画竖线
		for (GMsize_t i = 0; i < sliceN + 1; ++i)
		{
			GMVertex vert1 = { { x, y, z_start } };
			vert1.color = { desc.lineColor[0], desc.lineColor[1], desc.lineColor[2], 1 };
			vertices.push_back(std::move(vert1));

			GMVertex vert2 = { { x, y, z_end } };
			vert1.color = { desc.lineColor[0], desc.lineColor[1], desc.lineColor[2], 1 };
			vertices.push_back(std::move(vert2));

			x += dx;
		}

		/*
		for (GMsize_t j = 0; j < sliceM + 1; ++j)
		{

			z += dz;
			x = x_start;
		}
		*/

		GMModel* m = new GMModel();
		m->setUsageHint(GMUsageHint::DynamicDraw);
		m->getShader().setVertexColorOp(GMS_VertexColorOp::Replace);

		GMPart* part = new GMPart(m);
		part->swap(vertices);
		m->setPrimitiveTopologyMode(GMTopologyMode::Lines);
		scene = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, m));
	}

	GMModelAsset utGetFirstModelFromScene(GMSceneAsset asset)
	{
		return asset.getScene()->getModels()[0];
	}

	GMShader& utGetFirstShaderFromScene(GMSceneAsset asset)
	{
		return utGetFirstModelFromScene(asset).getModel()->getShader();
	}
}