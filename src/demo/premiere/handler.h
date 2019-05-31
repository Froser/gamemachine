#ifndef __HANDLER_H__
#define __HANDLER_H__

#include <gamemachine.h>
#include "procedures.h"

using namespace gm;

class Handler : public IGameHandler, public IShaderLoadCallback
{
public:
	Handler(IWindow*);
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
};

#endif