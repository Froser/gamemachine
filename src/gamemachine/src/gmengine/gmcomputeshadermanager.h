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

typedef GMuint32 GMComputeToken;

GM_PRIVATE_OBJECT_UNALIGNED(GMComputeShaderManager)
{
	Map<const IRenderContext*, Vector<IComputeShaderProgram*>> shaders;
	IComputeShaderProgram* deleter = nullptr;
};

class GMComputeShaderManager
{
	GM_DECLARE_PRIVATE_NGO(GMComputeShaderManager)

protected:
	GMComputeShaderManager() = default;
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