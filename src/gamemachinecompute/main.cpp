#if GM_WINDOWS
#include <windows.h>
#endif

#include <gamemachine.h>
#if GM_USE_DX11
#include <gmdx11.h>
#endif
#include <gmgl.h>
#define STRINGIFY(x) #x

using namespace gm;

bool verify(GMint32* a, GMint32* b, GMint32 sz)
{
	for (GMint32 i = 0; i < sz; ++i)
	{
		if (a[i] != b[i])
			return false;
	}
	return true;
}

int main(int argc, char* argv[])
{	
	/************************************************************************/
	/* 初始化                                                               */
	/************************************************************************/
	GMRenderEnvironment env = GMRenderEnvironment::OpenGL;

	if (argc > 1 && GMString(argv[1]) == "-opengl")
		env = GMRenderEnvironment::OpenGL;
	else
		env = GMRenderEnvironment::DirectX11;

	IFactory* factory = nullptr;
	if (env == GMRenderEnvironment::OpenGL)
	{
		factory = new GMGLFactory();
	}
	else
	{
#if GM_USE_DX11
		if (GMQueryCapability(GMCapability::SupportDirectX11))
			factory = new GMDx11Factory();
		else
			factory = new GMGLFactory();
#else
		env = GMRenderEnvironment::OpenGL;
		factory = new GMGLFactory();
#endif
	}

	GMGameMachineDesc desc;
	desc.factory = factory;
	desc.renderEnvironment = env;
	desc.runningMode = GMGameMachineRunningMode::ComputeOnly;
	GM.init(desc);
	const IRenderContext* computeContext = GM.getComputeContext();
	IComputeShaderProgram* prog = nullptr;
	if (!GM.getFactory()->createComputeShaderProgram(computeContext, &prog))
	{
		printf("Compute shader is not supported.");
		return -1;
	}

	/************************************************************************/
	/* 生成很多个大数                                                        */
	/************************************************************************/
	constexpr GMint32 sz = 10000000;
	printf("GameMachine GPU Compute Demo.\r\n");
	printf("CPU is creating %d random numbers...\r\n", sz);
	GMint32* a = new GMint32[sz];
	GMint32* b = new GMint32[sz];
	GMStopwatch stopwatch;
	stopwatch.start();
	for (auto i = 0; i < sz; ++i)
	{
		a[i] = GMRandomMt19937::random_int(0, sz);
		b[i] = GMRandomMt19937::random_int(0, sz);
	}
	stopwatch.stop();
	printf("Creating %d random numbers. Elapsed: %f\r\n", sz, stopwatch.timeInSecond());

	/************************************************************************/
	/* 使用CPU做一系列运算                                                    */
	/************************************************************************/
	stopwatch.start();
	GMint32* c = new GMint32[sz];
	for (auto i = 0; i < sz; ++i)
	{
		c[i] = a[i] * b[i];
	}
	stopwatch.stop();
	GMfloat CPUElapsed = stopwatch.timeInSecond();
	printf("CPU operation finished. Elapsed: %f\r\n", stopwatch.timeInSecond());

	/************************************************************************/
	/* 创建GPU ComputeShader 来计算                                          */
	/************************************************************************/
	auto glcs = STRINGIFY(
		#version 430 core\n

		layout(local_size_x = 16, local_size_y = 16) in; \n

		layout(std430, binding = 0) buffer A\n
		{ \n
			int a[]; \n
		}; \n

		layout(std430, binding = 1) buffer B\n
		{ \n
			int b[];
		}; \n

		layout(std430, binding = 2) buffer C\n
		{ \n
			int result[]; \n
		}; \n

		void main(void)\n
		{ \n
			uint gid = gl_GlobalInvocationID.x; \n
			result[gid] = a[gid] * b[gid]; \n
		}
	);

	auto dxcs = STRINGIFY(
		struct A { int a; };
		struct B { int b; };
		struct R { int value; };
		StructuredBuffer<A> BufferA : register(t0);
		StructuredBuffer<B> BufferB : register(t1);
		RWStructuredBuffer<R> Result : register(u0);

		[numthreads(1, 1, 1)]\n
		void main(uint3 DTid : SV_DispatchThreadID)\n
		{ \n
			Result[DTid.x].value = BufferA[DTid.x].a * BufferB[DTid.x].b; \n
		}
	);

	if (env == GMRenderEnvironment::OpenGL)
		prog->load(".", glcs, "main");
	else
		prog->load(".", dxcs, "main");

	GMComputeBufferHandle bufferA, bufferB, bufferResult;
	GMComputeSRVHandle srvA, srvB;
	GMComputeUAVHandle uav;
	prog->createBuffer(sizeof(GMint32), sz, a, GMComputeBufferType::Structured, &bufferA);
	prog->createBuffer(sizeof(GMint32), sz, b, GMComputeBufferType::Structured, &bufferB);
	prog->createBuffer(sizeof(GMint32), sz, c, GMComputeBufferType::UnorderedStructured, &bufferResult);
	prog->createBufferShaderResourceView(bufferA, &srvA);
	prog->createBufferShaderResourceView(bufferB, &srvB);
	prog->createBufferUnorderedAccessView(bufferResult, &uav);
	prog->bindShaderResourceView(1, &srvA);
	prog->bindShaderResourceView(1, &srvB);
	prog->bindUnorderedAccessView(1, &uav);

	stopwatch.start();
	prog->dispatch(16, 1, 1);
	stopwatch.stop();
	printf("GPU operation finished. Elapsed: %f\r\n", stopwatch.timeInSecond());

	printf("Verifing...\r\n");
	bool correct = false;

	stopwatch.start();
	bool canReadFromGPU = prog->canRead(bufferResult);
	GMint32* resultPtr = nullptr;
	if (canReadFromGPU)
	{
		resultPtr = static_cast<GMint32*>(prog->mapBuffer(bufferResult));
		correct = verify(c, resultPtr, sz);
		prog->unmapBuffer(bufferResult);
	}
	else
	{
		GMComputeBufferHandle cpuResult;
		prog->createReadOnlyBufferFrom(bufferResult, &cpuResult);
		prog->copyBuffer(cpuResult, bufferResult);
		resultPtr = static_cast<GMint32*>(prog->mapBuffer(cpuResult));
		correct = verify(c, resultPtr, sz);
		prog->unmapBuffer(cpuResult);
		prog->release(cpuResult);
	}
	stopwatch.stop();
	printf("Verifing finished. Elapsed: %f. Result: %s.\r\n", stopwatch.timeInSecond(), correct ? "Correct" : "Incorrect");

	prog->release(bufferA);
	prog->release(bufferB);
	prog->release(bufferResult);
	prog->release(srvA);
	prog->release(srvB);
	prog->release(uav);

	delete prog;
	delete[] a;
	delete[] b;
	delete[] c;
	getchar();
	GM.finalize();
	return 0;
}
