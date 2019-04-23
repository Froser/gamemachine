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
	if (targetContainter.size() <= i)
		targetContainter.resize(i + 1);

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

GMComputeShaderManager& GMComputeShaderManager::instance()
{
	static GMComputeShaderManager s_mgr;
	return s_mgr;
}
