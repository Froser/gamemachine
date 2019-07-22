#ifndef __HANDLER_H__
#define __HANDLER_H__

#include <gamemachine.h>
#include "procedures.h"

using namespace gm;

struct Config
{
	GMString windowName = L"GameMachine Premiere";
	GMint32 windowWidth = 1024;
	GMint32 windowHeight = 768;
	GMint32 samples = 0;
	GMString fontCN = L"simhei.ttf";
	GMString fontEN = L"suigener.ttf";
};

class Handler : public IGameHandler, public IShaderLoadCallback
{
public:
	Handler(IWindow*, Config&&) GM_NOEXCEPT;
	~Handler();

public:
	GMGameWorld* getWorld();
	IWindow* getWindow();

public:
	GMfloat getTick();

	// IShaderLoadCallback
private:
	virtual void onLoadShaders(const IRenderContext* context) override;

	// IGameHandler
private:
	virtual void init(const IRenderContext* context) override;
	virtual void start() override;
	virtual void event(GameMachineHandlerEvent evt) override;

private:
	void updateTick();

private:
	GMOwnedPtr<GMGameWorld> m_world;
	IWindow* m_mainWindow;
	GMOwnedPtr<Procedures> m_procedures;
	GMClock m_clock;
	Config m_config;
};

#endif