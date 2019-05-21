#ifndef __GMDX11_HELPER_H__
#define __GMDX11_HELPER_H__
#include <gmcommon.h>
#include <gmdxincludes.h>

BEGIN_NS
//! Direct11的辅助类。
/*!
  此类封装了很多Direct11的方法。每个方法都是以静态的形式访问。
*/
struct GMDx11Helper
{
	//! 从GM的包中读取一个着色器文件，并编译到DirectX11环境。
	/*!
	  函数将会首先尝试在prefetch文件夹获取预编译的文件，如果不匹配则尝试查找FXC来生成预编译文件，如果仍然失败则在运行时编译着色器。
	  调用此函数时，确认GMGamePackage已经初始化。如果读取失败，或者着色器文件编译失败，则返回E_FAIL。
	  \param engine 需要添加到的绘制引擎
	  \param code 着色器文件内容。
	  \param filepath 包含此着色器文件的路径。
	  \param gfxCandidate 预编译文件，如果为空则跳过尝试读取预编译的过程。
	  \return 操作是否成功。
	*/
	static IShaderProgram* loadEffectShader(
		IGraphicEngine* engine,
		const GMString& code,
		const GMString& filepath,
		const GMString& gfxCandidate
	);

	static void loadShader(
		const IRenderContext* context,
		const GMString& effectFilePath,
		const GMString& gfxCandidate
	);

	//! 返回一个默认的采样器描述。
	/*!
	  \return 默认采样器描述。
	*/
	static const D3D11_SAMPLER_DESC& getDefaultSamplerDesc();

	static D3D11_FILTER getTextureFilter(GMS_TextureFilter min, GMS_TextureFilter mag);
};

END_NS
#endif