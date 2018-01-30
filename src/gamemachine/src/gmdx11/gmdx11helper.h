#ifndef __GMDX11_HELPER_H__
#define __GMDX11_HELPER_H__
#include <gmcommon.h>

struct ID3D10Blob;

BEGIN_NS
extern "C"
{
	//! 从GM的包中读取一个着色器文件，并编译到DirectX11环境。
	/*!
	  调用此函数时，确认GMGamePackage已经初始化。如果读取失败，或者着色器文件编译失败，则返回E_FAIL。
	  \param filename 读取的文件名。此方法会通过GMGamePackage在着色器所在目录读取文件。
	  \param entryPoint 着色器程序的入口点。
	  \param profile 着色器配置。
	  \param out 着色器文件生成的缓存。
	  \return 操作是否成功。
	*/
	HRESULT GMLoadDx11Shader(
		const gm::GMString& filename,
		const gm::GMString& entryPoint,
		const gm::GMString& profile,
		GMShaderType type
	);
}

END_NS
#endif