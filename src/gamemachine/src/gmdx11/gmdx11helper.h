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
	  调用此函数时，确认GMGamePackage已经初始化。如果读取失败，或者着色器文件编译失败，则返回E_FAIL。
	  \param engine 需要添加到的绘制引擎
	  \param code 着色器文件内容。
	  \param filepath 包含此着色器文件的路径。
	  \param type 着色器类型，目前只能为GMShaderType::Effect。
	  \return 操作是否成功。
	*/
	static IShaderProgram* loadEffectShader(
		IGraphicEngine* engine,
		const GMString& code,
		const GMString& filepath
	);

	static void loadShader(
		const IRenderContext* context,
		const GMString& effectFilePath
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