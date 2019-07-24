#ifndef __GMCOMPUTESHADERMANAGER_H__
#define __GMCOMPUTESHADERMANAGER_H__
#include <gmcommon.h>
BEGIN_NS

enum GMComputeShader
{
	GMCS_GAMEOBJECT_CULL,
	GMCS_PARTICLE_GRAVITY,
	GMCS_PARTICLE_RADIAL,
	GMCS_PARTICLE_DATA_TRANSFER,
};

GM_PRIVATE_CLASS(GMComputeShaderManager);
class GMComputeShaderManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMComputeShaderManager)

protected:
	GMComputeShaderManager();
	~GMComputeShaderManager();

public:
	IComputeShaderProgram* getComputeShaderProgram(
		const IRenderContext* context,
		GMComputeShader i,
		const GMString& path,
		const GMString& code,
		const GMString& entryPoint
	);

	void disposeShaderPrograms(const IRenderContext*);
	void releaseHandle(GMComputeHandle handle);

public:
	static GMComputeShaderManager& instance();
};

END_NS
#endif