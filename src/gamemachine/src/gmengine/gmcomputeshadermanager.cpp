#include "stdafx.h"
#include <gamemachine.h>
#include "gmcomputeshadermanager.h"

GMComputeShaderManager::~GMComputeShaderManager()
{
	D(d);
	for (auto iter = d->shaders.begin(); iter != d->shaders.end(); ++iter)
	{
		disposeShaderPrograms(iter->first);
	}

	if (d->deleter)
		GM_delete(d->deleter);
}

IComputeShaderProgram* GMComputeShaderManager::getComputeShaderProgram(
	const IRenderContext* context,
	GMComputeShader i,
	const GMString& path,
	const GMString& code,
	const GMString& entryPoint
)
{
	D(d);
	auto& targetContainter = d->shaders[context];
	if (gm_sizet_to_int(targetContainter.size()) <= i)
		targetContainter.resize(i + 1);

	if (code.isEmpty())
		return nullptr;

	auto& prog = targetContainter[i];
	if (!prog)
	{
		if (GM.getFactory()->createComputeShaderProgram(context, &prog))
		{
			prog->load(path, code, entryPoint);
		}
	}
	return prog;
}

void GMComputeShaderManager::disposeShaderPrograms(const IRenderContext* context)
{
	D(d);
	auto& shaders = d->shaders[context];
	for (auto& s : shaders)
	{
		if (s)
		{
			GM_delete(s);
			s = nullptr;
		}
	}
	shaders.clear();
}


void GMComputeShaderManager::releaseHandle(GMComputeBufferHandle handle)
{
	D(d);
	if (!d->deleter && GM.getFactory())
		GM.getFactory()->createComputeShaderProgram(nullptr, &d->deleter);
	
	if (d->deleter)
	{
		d->deleter->release(handle);
	}
}

GMComputeShaderManager& GMComputeShaderManager::instance()
{
	static GMComputeShaderManager s_mgr;
	return s_mgr;
}
