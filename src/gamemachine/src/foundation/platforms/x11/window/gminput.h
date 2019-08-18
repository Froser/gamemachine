#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gminput.h>

BEGIN_NS

class JoystickStateImpl;
class MouseStateImpl;
class KeyboardStateImpl;

GM_PRIVATE_CLASS(GMInput);
class GMInput : public IInput
{
	GM_DECLARE_PRIVATE(GMInput)
	GM_DISABLE_COPY_ASSIGN(GMInput)
	GM_FRIEND_CLASS(JoystickStateImpl)
	GM_FRIEND_CLASS(MouseStateImpl)
	GM_FRIEND_CLASS(KeyboardStateImpl)

public:
	GMInput(IWindow* window);
	~GMInput();

	// IInput
public:
	// 每一帧，应该调用一次update
	virtual void update() override;
	virtual IKeyboardState& getKeyboardState() override;
	virtual IJoystickState& getJoystickState() override;
	virtual IMouseState& getMouseState() override;
	virtual IIMState& getIMState() override;
	virtual void handleSystemEvent(GMSystemEvent* event) override;

private:
	void recordMouseDown(GMMouseButton button);
	void recordMouseUp(GMMouseButton button);
	void recordWheel(bool wheeled, GMshort delta);
	void recordMouseMove();
};

END_NS

#endif