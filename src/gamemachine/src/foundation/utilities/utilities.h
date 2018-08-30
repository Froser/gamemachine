#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include <gmcommon.h>
#include <gmmodel.h>
#include <gmasync.h>

BEGIN_NS

class GMModel;
class GMShader;
class GMGameObject;
class GMParticleSystem;

GM_INTERFACE(IPrimitiveCreatorShaderCallback)
{
	virtual void onCreateShader(GMShader& shader) = 0;
};

//! 表示描述一个地形的结构
struct GMTerrainDescription
{
	const GMbyte* data; //!< 地形数据，如果为空，生成的高度均为0
	GMsize_t dataStride; //!< 相邻地形数据的间隔。比如，对于RGBA 32bits的高度图，只需要取R通道作为高度图，因此dataStride为4。
	GMint dataWidth; //!< 地形数据的宽度。
	GMint dataHeight; //!< 地形数据的高度。
	GMfloat terrainX; //!< 地形在x轴的起始位置。
	GMfloat terrainZ; //!< 地形在z轴的起始位置。
	GMfloat terrainLength; //!< 地形在x轴的长度。
	GMfloat terrainWidth; //!< 地形在y轴的长度。
	GMfloat heightScaling; //!< 地形高度的缩放比例。
	GMsize_t sliceM; //!< 地形在x轴的分块数。
	GMsize_t sliceN; //!< 地形在z轴的分块数。
	GMfloat textureLength; //!< 一块地形纹理在x轴的长度。
	GMfloat textureHeight; //!< 一块地形纹理在z轴的长度。
};

struct GMPrimitiveCreator
{
	enum GMCreateAnchor
	{
		TopLeft,
		Center,
	};

	static const GMVec3& one3();
	static const GMVec2& one2();
	static GMfloat* origin();

	static void createCube(const GMVec3& halfExtents, REF GMModelAsset& model);
	static void createQuadrangle(const GMVec2& halfExtents, GMfloat z, REF GMModelAsset& model);
	static void createSphere(GMfloat radius, GMint segmentsX, GMint segmentsY, REF GMModelAsset& model);

	static void createTerrain(
		const GMTerrainDescription& desc,
		REF GMModelAsset& model
	);

	static void createQuad3D(GMfloat extents[3], GMfloat position[12], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D, GMfloat(*customUV)[8] = nullptr);
};

//! 引擎提供的各种繁琐操作的工具类。
/*!
  此工具类其实是对一些固有流程的方法调用进行了封装，和单独调用那些方法效果一样。
*/
struct GMToolUtil
{
	//! 创建一个纹理，它来源于某路径。返回一个该纹理的资产。
	/*!
	  此方法封装了纹理读取和添加纹理等方法。请在调用前确认GMGamePackage已经初始化。
	  \param context 当前绘制环境上下文。
	  \param filename 需要读取纹理的路径。它是一个相对于纹理目录的相对路径。
	  \param width 纹理首层的宽度。可以为空。
	  \param height 纹理首层的高度。可以为空。
	  \sa GMGamePackage
	*/
	static GMTextureAsset createTexture(const IRenderContext* context, const GMString& filename, REF GMint* width = nullptr, REF GMint* height = nullptr);

	static void beginCreateTexture(const IRenderContext* context, const GMString& filename, GMAsyncCallback callback, OUT GMAsyncResult** ar);

	static void createTextureFromFullPath(const IRenderContext* context, const GMString& filename, REF GMTextureAsset& texture, REF GMint* width = nullptr, REF GMint* height = nullptr);

	//! 将一个纹理添加到一个模型中。
	/*!
	  此方法会将模型添加到纹理动画列表的第1帧中。
	  \param shader 目标模型的着色器设置。
	  \param texture 待添加纹理资产，它必须是纹理类型。
	  \param type 纹理类型。
	*/
	static void addTextureToShader(gm::GMShader& shader, GMAsset texture, GMTextureType type);

	static bool createPBRTextures(
		const IRenderContext* context,
		const GMString& albedoPath,
		const GMString& metallicPath,
		const GMString& roughnessPath,
		const GMString& aoPath,
		const GMString& normalPath,
		REF GMTextureAsset& albedoTexture,
		REF GMTextureAsset& metallicRoughnessAoTexture,
		REF GMTextureAsset& normalTexture
	);

	static void createCocos2DParticleSystem(const GMString& filename, OUT GMParticleSystem** particleSystem);
};

END_NS
#endif