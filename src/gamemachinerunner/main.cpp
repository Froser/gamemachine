#if GM_WINDOWS
#include <windows.h>
#endif

#include <wrapper.h>
#include <gamemachine.h>
#include <gmgl.h>
#include <gmlua.h>
#include <iostream>
using namespace gm;

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

	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::DirectX11;
	if (args.size() >= 3)
	{
		if (args[2] == L"-opengl")
			env = gm::GMRenderEnvironment::OpenGL;
	}

	// 初始化
	GMGameMachineDesc desc;
	env = GMCreateFactory(env, GMRenderEnvironment::OpenGL, &desc.factory);
	desc.renderEnvironment = env;
	
	GM.init(desc);

	// 读取Lua
	GMLua lua;
	GMLuaResult result = lua.runFile(args[1].toStdString().c_str());
	if (result.state != GMLuaStates::Ok)
	{
		gm_error("Syntax error: {0}", result.message);
		getchar();
	}

	return 0;
}
