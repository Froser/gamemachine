#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include <gmcommon.h>
#include <gmmodel.h>
BEGIN_NS

class GMModel;
class GMShader;
class GMGameObject;
class GMParticleSystem;

GM_INTERFACE(IPrimitiveCreatorShaderCallback)
{
	virtual void onCreateShader(GMShader& shader) = 0;
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

	static void createCube(const GMVec3& halfExtents, OUT GMModel** out);
	static void createQuadrangle(const GMVec2& halfExtents, GMfloat z, OUT GMModel** out);
	static void createSphere(GMfloat radius, GMint segmentsX, GMint segmentsY, OUT GMModel** out);

	static void createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D, GMCreateAnchor anchor = Center, GMfloat (*customUV)[12] = nullptr);
	static void createQuad3D(GMfloat extents[3], GMfloat position[12], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D, GMfloat(*customUV)[8] = nullptr);
};

//! 引擎提供的各种繁琐操作的工具类。
/*!
  此工具类其实是对一些固有流程的方法调用进行了封装，和单独调用那些方法效果一样。
*/
struct GMToolUtil
{
	//! 创建一个纹理，它来源于某路径。
	/*!
	  此方法封装了纹理读取和添加纹理等方法。请在调用前确认GMGamePackage已经初始化。
	  \param context 当前绘制环境上下文。
	  \param filename 需要读取纹理的路径。它是一个相对于纹理目录的相对路径。
	  \param texture 得到的纹理将通过此指针返回。
	  \param width 纹理首层的宽度。可以为空。
	  \param height 纹理首层的高度。可以为空。
	  \sa GMGamePackage
	*/
	static void createTexture(const IRenderContext* context, const GMString& filename, OUT ITexture** texture, REF GMint* width = nullptr, REF GMint* height = nullptr);

	static void createTextureFromFullPath(const IRenderContext* context, const GMString& filename, OUT ITexture** texture, REF GMint* width = nullptr, REF GMint* height = nullptr);

	//! 将一个纹理添加到一个模型中。
	/*!
	  此方法会将模型添加到纹理动画列表的第1帧中。
	  \param shader 目标模型的着色器设置。
	  \param texture 待添加纹理。
	  \param type 纹理类型。
	*/
	static void addTextureToShader(gm::GMShader& shader, ITexture* texture, GMTextureType type);

	static bool createPBRTextures(
		const IRenderContext* context,
		const GMString& albedoPath,
		const GMString& metallicPath,
		const GMString& roughnessPath,
		const GMString& aoPath,
		const GMString& normalPath,
		OUT ITexture** albedoTexture,
		OUT ITexture** metallicRoughnessAoTexture,
		OUT ITexture** normalTexture
	);

	static void createCocos2DParticleSystem(const GMString& filename, OUT GMParticleSystem** particleSystem);
};

END_NS
#endif