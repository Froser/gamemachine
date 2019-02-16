#include "stdafx.h"
#include <windows.h>
#include <gamemachine.h>
#include <gmgl.h>
#if GM_USE_DX11
#include <gmdx11.h>
#include <gmdx11helper.h>
#endif
#include <gmutilities.h>
#include "resource.h"
#include <commctrl.h>

using namespace gm;

#ifndef GetWindowOwner
#	define GetWindowOwner(hwnd) GetWindow(hwnd, GW_OWNER)
#endif

#ifndef GetWindowStyle
#	define GetWindowStyle(hwnd) ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#endif

namespace
{
	gm::GMRenderEnvironment s_env;

	void SetRenderEnv(gm::GMRenderEnvironment env)
	{
		s_env = env;
	}

	gm::GMRenderEnvironment GetRenderEnv()
	{
		return s_env;
	}

	HWND g_hDlg = NULL;

	INT_PTR CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND CreateMainDialog(HINSTANCE hInstance)
	{
		HWND hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1),
			NULL, &WndProc);
		return hDlg;
	}

	void MoveToCenter(HWND hWnd)
	{
		GM_ASSERT(::IsWindow(hWnd));
		GM_ASSERT((GetWindowStyle(hWnd)&WS_CHILD) == 0);
		RECT rcDlg = { 0 };
		::GetWindowRect(hWnd, &rcDlg);
		RECT rcArea = { 0 };
		RECT rcCenter = { 0 };
		HWND hWndParent = ::GetParent(hWnd);
		HWND hWndCenter = ::GetWindowOwner(hWnd);
		if (hWndCenter != NULL)
			hWnd = hWndCenter;

		// 处理多显示器模式下屏幕居中
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
		rcArea = oMonitor.rcWork;

		if (hWndCenter == NULL || IsIconic(hWndCenter))
			rcCenter = rcArea;
		else
			::GetWindowRect(hWndCenter, &rcCenter);

		int DlgWidth = rcDlg.right - rcDlg.left;
		int DlgHeight = rcDlg.bottom - rcDlg.top;

		// Find dialog's upper left based on rcCenter
		int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
		int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

		// The dialog is outside the screen, move it inside
		if (xLeft < rcArea.left) xLeft = rcArea.left;
		else if (xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
		if (yTop < rcArea.top) yTop = rcArea.top;
		else if (yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
		::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void InitUI(HWND hDlg)
	{
		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGEMIN, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_SETRANGEMAX, TRUE, 360);
	}

	//////////////////////////////////////////////////////////////////////////
	class FlowHandler : public IGameHandler, public IShaderLoadCallback
	{
	public:
		~FlowHandler();

	public:
		virtual void init(const IRenderContext* context);
		virtual void start();
		virtual void event(GameMachineHandlerEvent evt);
		virtual void onLoadShaders(const IRenderContext* context) override;

	public:
		void setRotation(GMfloat degree);
		const IRenderContext* getContext()
		{
			return m_context;
		}

	private:
		const IRenderContext* m_context = nullptr;
		GMGameWorld* m_world = nullptr;
		GMGameObject* m_obj = nullptr;
		GMSceneAsset m_cubeScene;
	};

	FlowHandler::~FlowHandler()
	{
		GM_delete(m_world);
	}

	void FlowHandler::init(const IRenderContext* context)
	{
		m_context = context;
		context->getEngine()->setShaderLoadCallback(this);
	}

	void FlowHandler::start()
	{
		gm::GMCamera& camera = m_context->getEngine()->getCamera();
		camera.setPerspective(Radian(75.f), 1.333f, .1f, 3200);

		gm::GMCameraLookAt lookAt;
		lookAt.lookAt = { 0, -.5f, 1 };
		lookAt.position = { 0, .5f, -1 };
		camera.lookAt(lookAt);

		m_world = new GMGameWorld(m_context);
		GMPrimitiveCreator::createCube(GMVec3(.2f, .2f, .2f), m_cubeScene);
		m_obj = new GMGameObject(m_cubeScene);
		m_world->addObjectAndInit(m_obj);
		m_world->addToRenderList(m_obj);

		GM.renderFrame(m_context->getWindow());
	}

	void FlowHandler::onLoadShaders(const IRenderContext* context)
	{
		static const char* s_gl_vs =
			"#version 330\n"
			"layout (location = 0) in vec3 gm_position;\n"
			"layout (location = 1) in vec3 gm_normal;\n"
			"\n"
			"vec4 position;\n"
			"vec4 normal;\n"
			"out vec4 _position;\n"
			"out vec4 _normal;\n"
			"\n"
			"uniform mat4 GM_ViewMatrix;\n"
			"uniform mat4 GM_WorldMatrix;\n"
			"uniform mat4 GM_ProjectionMatrix;\n"
			"\n"
			"void init_layouts()\n"
			"{\n"
			"	position = vec4(gm_position.xyz, 1);\n"
			"	normal = normalize(vec4(mat3(GM_WorldMatrix) * gm_normal.xyz, 1));\n"
			"}\n"
			"\n"
			"void main()\n"
			"{\n"
			"	init_layouts();\n"
			"\n"
			"	gl_Position = GM_ProjectionMatrix * GM_ViewMatrix * GM_WorldMatrix * position;\n"
			"	_position = position;\n"
			"	_normal = normal;\n"
			"}\n"
			;
		static const char* s_gl_fs =
			"in vec4 _position;\n"
			"in vec4 _normal;\n"
			"\n"
			"uniform mat4 GM_ViewMatrix;\n"
			"uniform mat4 GM_WorldMatrix;\n"
			"vec3 lightDirection_N = normalize(vec3(2, 1, 1));\n"
			"vec3 lightColor = vec3(0, .5, .5);\n"
			"vec3 ambient() { return lightColor; }\n"
			"vec3 diffuse() {\n"
			"	vec3 eyeDirection_world = (GM_WorldMatrix * _position).xyz;\n"
			"	vec3 eyeDirection_world_N = normalize(eyeDirection_world);\n"
			"	vec3 lightPosition_world = (vec4(lightDirection_N, 1)).xyz;\n"
			"	vec3 lightDirection_world_N = normalize(lightPosition_world + eyeDirection_world);\n"
			"	return max(dot(lightDirection_world_N, _normal.xyz) * lightColor, 0);\n"
			"}\n"
			"\n"
			"void main() {\n"
			"	gl_FragColor = vec4(ambient() + diffuse(), 1);\n"
			"}\n"
			;

#if GM_USE_DX11
		static const char* s_eff =
			"cbuffer WorldConstantBuffer: register( b0 ) \n"
			"{\n"
			"	matrix GM_WorldMatrix;\n"
			"	matrix GM_ViewMatrix;\n"
			"	matrix GM_ProjectionMatrix;\n"
			"}\n"
			"RasterizerState GM_RasterizerState{};\n"
			"BlendState GM_BlendState{};\n"
			"DepthStencilState GM_DepthStencilState{};\n"
			"\n"
			"float3x3 ToFloat3x3(float4x4 m)\n"
			"{\n"
			"	return float3x3(\n"
			"		m[0].xyz,\n"
			"		m[1].xyz,\n"
			"		m[2].xyz\n"
			"	);\n"
			"}\n"
			"struct VS_INPUT\n"
			"{\n"
			"	float3 Position    : POSITION;\n"
			"	float3 Normal      : NORMAL0;\n"
			"};\n"
			"\n"
			"struct VS_OUTPUT\n"
			"{\n"
			"	float3 Normal      : NORMAL0;\n"
			"	float4 WorldPos    : POSITION;\n"
			"	float4 Position    : SV_POSITION;\n"
			"};\n"
			"\n"
			"VS_OUTPUT VS_3D( VS_INPUT input )\n"
			"{\n"
			"	VS_OUTPUT output;\n"
			"	output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1);\n"
			"\n"
			"	output.Position = mul(output.Position, GM_WorldMatrix);\n"
			"	output.WorldPos = output.Position;\n"
			"	\n"
			"	output.Position = mul(output.Position, GM_ViewMatrix);\n"
			"	output.Position = mul(output.Position, GM_ProjectionMatrix);\n"
			"\n"
			"	output.Normal = normalize(mul(input.Normal, ToFloat3x3(GM_WorldMatrix)));\n"
			"	return output;\n"
			"}\n"
			"\n"
			"typedef VS_OUTPUT PS_INPUT;\n"
			"\n"
			"float3 lightDirection_N = normalize(float3(2, 1, 1));\n"
			"float3 lightColor = float3(0, .5, .5);\n"
			"float3 ambient()\n"
			"{\n"
			"	return lightColor;\n"
			"}\n"
			"\n"
			"float3 diffuse(PS_INPUT input)\n"
			"{\n"
			"	float3 eyeDirection_world_N = normalize((input.WorldPos).xyz);\n"
			"	float3 lightDirection_world_N = normalize(float4(lightDirection_N.x, lightDirection_N.y, lightDirection_N.z, 1).xyz); \n"
			"	return max(dot(lightDirection_world_N, input.Normal) * lightColor, 0);\n"
			"}\n"
			"\n"
			"float4 PS_3D(PS_INPUT input) : SV_TARGET\n"
			"{\n"
			"	float3 result = ambient() + diffuse(input);\n"
			"	return float4(result.x, result.y, result.z, 1);\n"
			"}\n"
			"\n"
			"technique11 GMTech_3D\n"
			"{\n"
			"	pass P0\n"
			"	{\n"
			"		SetVertexShader(CompileShader(vs_5_0,VS_3D()));\n"
			"		SetPixelShader(CompileShader(ps_5_0,PS_3D()));\n"
			"		SetRasterizerState(GM_RasterizerState); \n"
			"		SetBlendState(GM_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF); \n"
			"		SetDepthStencilState(GM_DepthStencilState, 1); \n"
			"	}\n"
			"}\n"
			;
#endif

		if (GM.getRunningStates().renderEnvironment == GMRenderEnvironment::OpenGL)
		{
			GMGLShaderProgram* program = new GMGLShaderProgram(context);
			GMGLShaderInfo shadersInfo[] = {
				{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Vertex), s_gl_vs, "" },
				{ GMGLShaderInfo::toGLShaderType(gm::GMShaderType::Pixel), s_gl_fs, "" },
			};
			program->attachShader(shadersInfo[0]);
			program->attachShader(shadersInfo[1]);
			program->load();
			context->getEngine()->setInterface(gm::GameMachineInterfaceID::GLForwardShaderProgram, program);
		}
		else
		{
#if GM_USE_DX11
			GMRenderTechniques techs;
			GMRenderTechnique tech(GMShaderType::Effect);
			tech.setCode(GMRenderEnvironment::DirectX11, GMString(s_eff));
			tech.setPath(GMRenderEnvironment::DirectX11, "temp");
			techs.addRenderTechnique(tech);
			context->getEngine()->getRenderTechniqueManager()->addRenderTechniques(techs);
#endif
		}
	}

	void FlowHandler::setRotation(GMfloat degree)
	{
		if (m_obj)
		{
			m_obj->setRotation(Rotate(Radian(degree), GMVec3(0, 1, 0)));
		}
	}

	void FlowHandler::event(GameMachineHandlerEvent evt)
	{
		switch (evt)
		{
		case GameMachineHandlerEvent::Render:
		{
			m_context->getEngine()->getDefaultFramebuffers()->clear();
			m_world->renderScene();
			break;
		}
		}
	}

	FlowHandler* s_flow = nullptr;
	//////////////////////////////////////////////////////////////////////////

	IWindow* CreateGameMachineChildWindow(HINSTANCE hInstance, HWND hContainer, IFactory* pFactory)
	{
		IWindow* pWindow = NULL;
		pFactory->createWindow(hInstance, 0, &pWindow);
		GM_ASSERT(pWindow);

		pWindow->setHandler(s_flow = new FlowHandler());

		GMWindowDesc wndAttrs;
		wndAttrs.createNewWindow = false;
		wndAttrs.existWindowHandle = hContainer;
		pWindow->create(wndAttrs);
		return pWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	INT_PTR CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_QUIT:
		case WM_CLOSE:
		{
			GM.exit();
			return TRUE;
		}
		case WM_HSCROLL:
		{
			if (GetDlgItem(g_hDlg, IDC_SLIDER1) == (HWND)lParam)
			{
				INT angle = (INT)SendDlgItemMessage(g_hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
				s_flow->setRotation(-angle);
				HWND hContainer = GetDlgItem(g_hDlg, ID_CONTAINER);
				GM.renderFrame(s_flow->getContext()->getWindow());
			}
			return TRUE;
		}
		case WM_PAINT:
		{
			if (s_flow)
				GM.renderFrame(s_flow->getContext()->getWindow());
			return TRUE;
		}
		}

		return FALSE;
	}
}

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
	// 这个例子中，我们不使用游戏模式，而将它作为一个窗口，并将它嵌入一个程序
	LPWSTR cmdLine = GetCommandLineW();
	gm::GMsize_t sz = gm::GMString::countOfCharacters(cmdLine);
	gm::GMRenderEnvironment env = gm::GMRenderEnvironment::DirectX11;
	if (sz > 7 && gm::GMString(cmdLine + sz - 7) == "-opengl")
		env = gm::GMRenderEnvironment::OpenGL;
	SetRenderEnv(env);

	HWND hDlg = CreateMainDialog(hInstance);
	MoveToCenter(hDlg);
	InitUI(hDlg);
	ShowWindow(hDlg, SW_SHOWNORMAL);
	UpdateWindow(hDlg);
	g_hDlg = hDlg;

	IFactory* pFactory = nullptr;
	if (GetRenderEnv() == gm::GMRenderEnvironment::OpenGL)
	{
		pFactory = new gm::GMGLFactory();
	}
	else
	{
#if GM_USE_DX11
		if (gm::GMQueryCapability(gm::GMCapability::SupportDirectX11))
			pFactory = new gm::GMDx11Factory();
		else
			pFactory = new gm::GMGLFactory();
#else
		SetRenderEnv(gm::GMRenderEnvironment::OpenGL);
		pFactory = new gm::GMGLFactory();
#endif
	}

	IWindow* pChildWnd = CreateGameMachineChildWindow(hInstance, GetDlgItem(hDlg, ID_CONTAINER), pFactory);
	GM.addWindow(pChildWnd);

	GMGameMachineDesc desc;
	desc.factory = pFactory;
	desc.renderEnvironment = GetRenderEnv();
	desc.runningMode = GMGameMachineRunningMode::ApplicationMode;
	GM.init(desc);

	GM.startGameMachine();
	return 0;
}
