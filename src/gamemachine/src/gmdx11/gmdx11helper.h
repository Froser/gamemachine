#ifndef __GMDX11_HELPER_H__
#define __GMDX11_HELPER_H__
#include <gmcommon.h>

struct ID3D10Blob;
struct D3D11_RASTERIZER_DESC;

BEGIN_NS
extern "C"
{
	//! 从GM的包中读取一个着色器文件，并编译到DirectX11环境。
	/*!
	  调用此函数时，确认GMGamePackage已经初始化。如果读取失败，或者着色器文件编译失败，则返回E_FAIL。
	  \param engine 需要添加到的绘制引擎
	  \param filename 读取的文件名。此方法会通过GMGamePackage在着色器所在目录读取文件。
	  \param entryPoint 着色器程序的入口点。
	  \param profile 着色器配置。
	  \param out 着色器文件生成的缓存。
	  \return 操作是否成功。
	*/
	HRESULT GMLoadDx11Shader(
		IGraphicEngine* engine,
		const gm::GMString& filename,
		const gm::GMString& entryPoint,
		const gm::GMString& profile,
		GMShaderType type
	);

	//! 返回一个默认的光栅描述结构体。
	/*!
	  GameMachine将使用此结构来初始化默认的DirectX光珊。
	  \param multisampleEnable 是否开启多重采样。
	  \param antialiasedLineEnable 是否开启抗锯齿。
	  \return 默认的光栅描述结构体。
	*/
	const D3D11_RASTERIZER_DESC& GMGetDefaultRasterizerDesc(
		bool multisampleEnable,
		bool antialiasedLineEnable
	);
}

END_NS
#endif