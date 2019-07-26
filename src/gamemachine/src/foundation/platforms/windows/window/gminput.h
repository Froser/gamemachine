#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gminput.h>

BEGIN_NS

class GMXInputWrapper
{
	typedef DWORD(WINAPI *XInputGetState_Delegate)(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef DWORD(WINAPI *XInputSetState_Delegate)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

public:
	GMXInputWrapper();
	~GMXInputWrapper();

public:
	DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
	DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

private:
	HMODULE m_module;
	XInputGetState_Delegate m_xinputGetState;
	XInputSetState_Delegate m_xinputSetState;
};

GM_PRIVATE_CLASS(GMInput);
class GMInput : public IInput
{
	GM_DECLARE_PRIVATE(GMInput)
	GM_DISABLE_COPY_ASSIGN(GMInput)

public:
	GMInput(IWindow* window);
	~GMInput();

public:
	// 每一帧，应该调用一次update
	virtual void update() override;
	virtual IKeyboardState& getKeyboardState() override;
	virtual IJoystickState& getJoystickState() override;
	virtual IMouseState& getMouseState() override;
	virtual IIMState& getIMState() override;
	virtual void handleSystemEvent(GMSystemEvent* event) override;

public:
	Data& dataRef();

private:
	void recordMouseDown(GMMouseButton button);
	void recordMouseUp(GMMouseButton button);
	void recordWheel(bool wheeled, GMshort delta);
	void recordMouseMove();
};

END_NS

#endif