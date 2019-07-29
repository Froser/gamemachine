#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include "gmengine/ui/gmwindow_p.h"
#include <GL/glew.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include "gmxrendercontext.h"
#include "gmgl/gmglgraphic_engine.h"

#define ATTRIB(a) attributes[where++]=(a)
#define ATTRIB_VAL(a, v) { ATTRIB(a); ATTRIB(v); }
#define MAX_ATTRIBS 100
#define ATTRIBS attributes
#define INIT_ATTRIBS() GMint32 attributes[MAX_ATTRIBS] = {0}; GMint32 where = 0;

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

BEGIN_NS

namespace
{
	void setAttributes(GMint32* attributes, const GMWindowDesc& wndAttrs, bool useSample)
	{
		int where = 0;
		ATTRIB_VAL(GLX_X_RENDERABLE, True);
		ATTRIB_VAL(GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT);
		ATTRIB_VAL(GLX_RENDER_TYPE, GLX_RGBA_BIT);
		ATTRIB_VAL(GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR);
		ATTRIB_VAL(GLX_RED_SIZE, 8);
		ATTRIB_VAL(GLX_GREEN_SIZE, 8);
		ATTRIB_VAL(GLX_BLUE_SIZE, 8);
		ATTRIB_VAL(GLX_ALPHA_SIZE, 8);
		ATTRIB_VAL(GLX_DOUBLEBUFFER, True);
		ATTRIB_VAL(GLX_DEPTH_SIZE, 24);
		ATTRIB_VAL(GLX_STENCIL_SIZE, 8);
		if (useSample && wndAttrs.samples > 1)
		{
			ATTRIB_VAL(GLX_SAMPLE_BUFFERS, 1);
			ATTRIB_VAL(GLX_SAMPLES, wndAttrs.samples);
		}
		ATTRIB(None);
	}

	bool chooseConfig(GLXFBConfig* config, const GMWindowDesc& wndAttrs, const GMXRenderContext* context)
	{
		INIT_ATTRIBS();
		setAttributes(ATTRIBS, wndAttrs, true);
		GLXFBConfig* fbconfigArray = NULL;
		GMint32 fbconfigArraySize = 0;

		fbconfigArray = glXChooseFBConfig(context->getDisplay(), context->getScreen(), ATTRIBS, &fbconfigArraySize);
		if (fbconfigArray)
		{
			*config = fbconfigArray[0];
			XFree(fbconfigArray);
			return true;
		}
		else
		{
			// Perhaps some attributes is not supported.
			setAttributes(ATTRIBS, wndAttrs, false);
			fbconfigArray = glXChooseFBConfig(context->getDisplay(), context->getScreen(), ATTRIBS, &fbconfigArraySize);
			if (fbconfigArray)
			{
				*config = fbconfigArray[0];
				XFree(fbconfigArray);
				return true;
			}
			else
			{
				gm_error(gm_dbg_wrap("Cannot get FBConfig."));
				return false;
			}
		}
	}

	Bool windowIsVisible_Predicator(Display*, XEvent* event, XPointer arg)
	{
		Window window = (Window)arg;
		return (event->type == MapNotify) && (event->xmap.window == window);
	}
}

class GMWindow_OpenGL;
GM_PRIVATE_OBJECT_UNALIGNED(GMWindow_OpenGL)
{
	GMWindow_OpenGL* parent = nullptr;
	GLXFBConfig fbConfig = nullptr;
	GMOwnedPtr<GMGLGraphicEngine> engine;
	GMOwnedPtr<IRenderContext> context;
};

class GMWindow_OpenGL : public GMWindow
{
	GM_DECLARE_PRIVATE(GMWindow_OpenGL)
	GM_DECLARE_BASE(GMWindow)

public:
	GMWindow_OpenGL(IWindow* parent);
	~GMWindow_OpenGL();

public:
	virtual IGraphicEngine* getGraphicEngine() override;
	virtual const IRenderContext* getContext() override;
	virtual void msgProc(const GMMessage& message) override;

protected:
	virtual void onWindowCreated(const GMWindowDesc& wndAttrs) override;

private:
	GLXContext createNewContext();
	void dispose();
	void createGraphicEngine();
};

GMWindow_OpenGL::GMWindow_OpenGL(IWindow* parent)
{
	GM_CREATE_DATA();
	D(d);
	if (parent)
	{
		GMWindow_OpenGL* parentGLWindow = dynamic_cast<GMWindow_OpenGL*>(parent);
		if (parentGLWindow)
			d->parent = parentGLWindow;
	}
}

GMWindow_OpenGL::~GMWindow_OpenGL()
{
	dispose();
}

void GMWindow_OpenGL::onWindowCreated(const GMWindowDesc& wndAttrs)
{
	D(d);
	D_BASE(db, Base);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	bool suc = chooseConfig(&d->fbConfig, wndAttrs, context);
	if (!suc)
	{
		gm_error(gm_dbg_wrap("Window Attribute is not supported."));
		return;
	}
	
	XVisualInfo* visualInfo = nullptr;
	visualInfo = glXGetVisualFromFBConfig(context->getDisplay(), d->fbConfig);
	GM_ASSERT(visualInfo);
	if (!visualInfo)
	{
		gm_error(gm_dbg_wrap("VisualInfo cannot be retrieved."));
		return;
	}

	XSetWindowAttributes winAttr;
	winAttr.event_mask =
		StructureNotifyMask | SubstructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask |
		KeyPressMask | KeyReleaseMask | VisibilityChangeMask | EnterWindowMask | LeaveWindowMask |
		PointerMotionMask | ButtonMotionMask | KeymapStateMask | FocusChangeMask;
	winAttr.background_pixmap = None;
	winAttr.background_pixel = 0;
	winAttr.border_pixel = 0;
	winAttr.colormap = XCreateColormap(context->getDisplay(), context->getRootWindow(), visualInfo->visual, AllocNone);

	// Create window and set handle
	Window window = 0;
	if (wndAttrs.createNewWindow)
	{
		GMulong mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | wndAttrs.dwStyle;
		window = XCreateWindow(
			context->getDisplay(),
			d->parent ? d->parent->getWindowHandle() : context->getRootWindow(),
			wndAttrs.rc.x,
			wndAttrs.rc.y,
			wndAttrs.rc.width,
			wndAttrs.rc.height,
			0,
			visualInfo->depth,
			InputOutput,
			visualInfo->visual,
			mask,
			&winAttr
		);
	}
	else
	{
		window = wndAttrs.existWindowHandle;
	}

	Display* display = context->getDisplay();
	setWindowHandle(window, true);

	db->windowStates.sampleCount = wndAttrs.samples;
	db->windowStates.sampleQuality = 0;

	// CreateContext
	GLXContext glxContext = createNewContext();
	const_cast<GMXRenderContext*>(context)->setGlxContext(glxContext);
	context->switchToContext();

	XSizeHints sizeHints;
	sizeHints.flags = USPosition | USSize;
	sizeHints.x = wndAttrs.rc.x;
	sizeHints.y = wndAttrs.rc.y;
	sizeHints.width = wndAttrs.rc.width;
	sizeHints.height = wndAttrs.rc.height;

	XWMHints wmHints;
	wmHints.flags = StateHint;
	wmHints.initial_state = NormalState;
	std::string title = wndAttrs.windowName.toStdString();
	const char* c_title = title.c_str();

	XTextProperty textProperty;
	XStringListToTextProperty( const_cast<char**>(&c_title), 1, &textProperty);
	XSetWMProperties(
		context->getDisplay(),
		getWindowHandle(),
		&textProperty,
		&textProperty,
		0,
		0,
		&sizeHints,
		&wmHints,
		NULL
	);
	XFree(textProperty.value);

	if (!XSetWMProtocols(context->getDisplay(), getWindowHandle(), const_cast<Atom*>(&context->getAtomDeleteWindow()), True))
		gm_error(gm_dbg_wrap("Cannot set atom protocol: 'WM_DELETE_WINDOW'"));

	if (context->getNetWMSupported()
		&& context->getAtomNetWMPid() != None
		&& context->getAtomClientMachine() != None
	)
	{
		char hostname[HOST_NAME_MAX];
		pid_t pid = getpid();
		if (pid > 0 && gethostname(hostname, sizeof(hostname)) > -1)
		{
			hostname[sizeof(hostname) - 1] = 0;
			XChangeProperty(
				context->getDisplay(),
				getWindowHandle(),
				context->getAtomNetWMPid(),
				XA_CARDINAL,
				32,
				PropModeReplace,
				(unsigned char *)&pid,
				1
			);

			XChangeProperty(
				context->getDisplay(),
				getWindowHandle(),
				context->getAtomClientMachine(),
				XA_STRING,
				8,
				PropModeReplace,
				(unsigned char *)hostname,
				strlen(hostname)
			);
		}
	}

	XMapWindow(context->getDisplay(), getWindowHandle());
	XFree(visualInfo);

	XEvent eventReturnBuffer;
	XPeekIfEvent(context->getDisplay(), &eventReturnBuffer, &windowIsVisible_Predicator, (XPointer)getWindowHandle());

	createGraphicEngine();
	glewInit();
}

GLXContext GMWindow_OpenGL::createNewContext()
{
	D(d);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	Display* display = context->getDisplay();
	GLXContext share_list = NULL;
	typedef GLXContext (*CreateContextAttribsProc) (Display*, GLXFBConfig, GLXContext, Bool, const int*);
	CreateContextAttribsProc createContextAttribs = (CreateContextAttribsProc) glXGetProcAddressARB((const GLubyte*)("glXCreateContextAttribsARB"));
	if (!createContextAttribs)
	{
		gm_error(gm_dbg_wrap("glXCreateContextAttribsARB not found. Terminated."));
		return NULL;
	}

	if (!createContextAttribs)
	{
		return glXCreateNewContext(display, d->fbConfig, GLX_RGBA_TYPE, share_list, true);
	}

	GMint32 attributes[9];
	GMint32 where = 0;
	GMint32 contextFlags;
#if GM_DEBUG
	contextFlags = GLX_CONTEXT_DEBUG_BIT_ARB | GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#else
	contextFlags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#endif

	// OpenGL 3.3
	ATTRIB_VAL(GLX_CONTEXT_MAJOR_VERSION_ARB, 3);
	ATTRIB_VAL(GLX_CONTEXT_MINOR_VERSION_ARB, 3);
	ATTRIB_VAL(GLX_CONTEXT_FLAGS_ARB, contextFlags);
	ATTRIB(None);

	GLXContext xcontext = createContextAttribs(display, d->fbConfig, share_list, true, attributes);
	if (!context)
	{
		gm_error(gm_dbg_wrap("GLXContext create failed."));
		return NULL;
	}

	return xcontext;
}

IGraphicEngine* GMWindow_OpenGL::getGraphicEngine()
{
	D_BASE(d, Base);
	if (!d->engine)
	{
		d->engine = gm_makeOwnedPtr<GMGLGraphicEngine>(getContext());
	}
	return d->engine.get();
}

const IRenderContext* GMWindow_OpenGL::getContext()
{
	D_BASE(d, Base);
	if (!d->context)
	{
		GMXRenderContext* context = new GMXRenderContext(this);
		d->context.reset(context);
		context->setWindow(this);
		
	}
	return d->context.get();
}

void GMWindow_OpenGL::dispose()
{
	D(d);
	GMWindowHandle wnd = getWindowHandle();
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	if (!d->parent && context && context->getGlxContext())
	{
		// 如果一个窗口有parent，则与parent使用相同的RC，因此RC由parent释放
		glXDestroyContext(context->getDisplay(), context->getGlxContext());
		d->context.reset(nullptr);
	}

	d->fbConfig = nullptr;
	if (getWindowHandle())
	{
		XDestroyWindow(context->getDisplay(), getWindowHandle());
		setWindowHandle(0, false);
	}
}

void GMWindow_OpenGL::createGraphicEngine()
{
	D(d);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	const_cast<GMXRenderContext*>(context)->setEngine(getGraphicEngine());
}

void GMWindow_OpenGL::msgProc(const GMMessage& message)
{
	Base::msgProc(message);

	D(d);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	if (message.msgType == GameMachineMessageType::FrameUpdate)
	{
		glXSwapBuffers(context->getDisplay(), getWindowHandle());
	}
}

bool GMWindowFactory::createWindowWithOpenGL(GMInstance instance, IWindow* parent, OUT IWindow** window)
{
	if (window)
	{
		(*window) = new GMWindow_OpenGL(parent);
		if (*window)
			return true;
	}
	return false;
}

bool GMWindowFactory::createTempWindow(GMbyte colorDepth, GMbyte alphaBits, GMbyte depthBits, GMbyte stencilBits, OUT GMWindowHandle& tmpWnd, OUT GMDeviceContextHandle& tmpDC, OUT GMOpenGLRenderContextHandle& tmpRC)
{
	return false;
}

bool GMWindowFactory::destroyTempWindow(GMWindowHandle tmpWnd, GMDeviceContextHandle tmpDC, GMOpenGLRenderContextHandle tmpRC)
{

}

END_NS