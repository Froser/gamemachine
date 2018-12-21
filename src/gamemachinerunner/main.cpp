#if GM_WINDOWS
#include <windows.h>
#endif

#include <gamemachine.h>
#include <gmgl.h>
#if GM_USE_DX11
#include <gmdx11.h>
#include <gmdx11helper.h>
#endif

#include <gmlua.h>
#include <iostream>
using namespace gm;

namespace
{
	gm::GMRenderEnvironment s_env;

	void SetRenderEnv(gm::GMRenderEnvironment env)
	{
		s_env = env;
	}
}

gm::GMRenderEnvironment GetRenderEnv()
{
	return s_env;
}

struct ConsoleDebugOutput : public IDebugOutput
{
	virtual void info(const GMString& msg) override
	{
		std::cout << msg.toStdString() << std::endl;
	}

	virtual void warning(const GMString& msg) override
	{
		std::cout << msg.toStdString() << std::endl;
	}

	virtual void error(const GMString& msg) override
	{
		std::cout << msg.toStdString() << std::endl;
	}

	virtual void debug(const GMString& msg) override
	{
		std::cout << msg.toStdString() << std::endl;
	}
} g_debugOutput;

int main(int argc, char* argv[])
{	
	GMDebugger::setDebugOutput(&g_debugOutput);

	Vector<GMString> args;
	for (int i = 0; i < argc; ++i)
	{
		args.push_back(GMString(argv[i]));
	}

	if (args.size() <= 1)
	{
		gm_error("Incorrect arguments.");
		return 0;
	}

#if GM_WINDOWS
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::DirectX11;
	if (args.size() >= 3)
	{
		if (args[2] == L"-opengl")
			env = gm::GMRenderEnvironment::OpenGL;
	}

#elif GM_UNIX
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::OpenGL;
#endif
	SetRenderEnv(env);

	// 初始化
	GMGameMachineDesc desc;
	desc.renderEnvironment = GetRenderEnv();
#if GM_USE_DX11
	if (GetRenderEnv() == GMRenderEnvironment::OpenGL)
		desc.factory = new GMGLFactory();
	else
		desc.factory = new GMDx11Factory();
#else
	desc.factory = new GMGLFactory();
#endif
	GM.init(desc);

	// 读取Lua
	GMLua lua;
	GMLuaResult result = lua.runFile(args[1].toStdString().c_str());
	if (result.state == GMLuaStates::SyntaxError)
	{
		gm_error("Syntax error: {0}", result.message);
		getchar();
	}

	return 0;
}
