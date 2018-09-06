#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
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
#define INIT_ATTRIBS() GMint attributes[MAX_ATTRIBS]; GMint where = 0;

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

namespace
{
	bool chooseConfig(GLXFBConfig* config, const GMWindowAttributes& wndAttrs, const GMXRenderContext* context)
	{
		INIT_ATTRIBS();
		ATTRIB_VAL(GLX_RED_SIZE, 1);
		ATTRIB_VAL(GLX_GREEN_SIZE, 1);
		ATTRIB_VAL(GLX_BLUE_SIZE, 1);
		ATTRIB_VAL(GLX_ALPHA_SIZE, 1);
		ATTRIB_VAL(GLX_DOUBLEBUFFER, True);
		ATTRIB_VAL(GLX_STEREO, True);
		ATTRIB_VAL(GLX_DEPTH_SIZE, 1);
		ATTRIB_VAL(GLX_STENCIL_SIZE, 1);

		if (wndAttrs.samples > 1)
		{
			ATTRIB_VAL(GLX_SAMPLE_BUFFERS, 1);
			ATTRIB_VAL(GLX_SAMPLES, wndAttrs.samples);
		}

		ATTRIB(None);

		GLXFBConfig* fbconfigArray;
		GMint fbconfigArraySize = 0;

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

	Bool windowIsVisible_Predicator(Display*, XEvent* event, XPointer arg)
	{
		Window window = (Window)arg;
		return (event->type == MapNotify) && (event->xmap.window == window);
	}
}

GM_PRIVATE_OBJECT(GMWindow_OpenGL)
{
	GMWindow_OpenGL* parent = nullptr;
	GLXFBConfig fbConfig = nullptr;
	GMOwnedPtr<GMGLGraphicEngine> engine;
	GMOwnedPtr<IRenderContext> context;
};

class GMWindow_OpenGL : public GMWindow
{
	GM_DECLARE_PRIVATE_AND_BASE(GMWindow_OpenGL, GMWindow)

public:
	GMWindow_OpenGL(IWindow* parent);
	~GMWindow_OpenGL();

public:
	virtual IGraphicEngine* getGraphicEngine() override;
	virtual const IRenderContext* getContext() override;
	virtual void onWindowCreated(const GMWindowAttributes& wndAttrs) override;

private:
	GLXContext createNewContext();
	void dispose();
};

GMWindow_OpenGL::GMWindow_OpenGL(IWindow* parent)
{
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

void GMWindow_OpenGL::onWindowCreated(const GMWindowAttributes& wndAttrs)
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
		PointerMotionMask | ButtonMotionMask;
	winAttr.background_pixmap = None;
	winAttr.background_pixel = 0;
	winAttr.border_pixel = 0;
	winAttr.colormap = XCreateColormap(context->getDisplay(), context->getRootWindow(), visualInfo->visual, AllocNone);

	// Create window and set handle
	GMulong mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | wndAttrs.dwStyle;
	Window window = XCreateWindow(
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

	Display* display = context->getDisplay();
	GMint renderType = GLX_RGBA_TYPE;
	GLXContext share_list = NULL;
	typedef GLXContext (*CreateContextAttribsProc) (Display*, GLXFBConfig, GLXContext, Bool, const int*);
	CreateContextAttribsProc createContextAttribs = (CreateContextAttribsProc) glXGetProcAddressARB((const GLubyte*)("glXCreateContextAttribsARB"));
	if (!createContextAttribs)
	{
		gm_error(gm_dbg_wrap("glXCreateContextAttribsARB not found. Terminated."));
		return;
	}

	setWindowHandle(window);

	// CreateContext
	GLXContext glxContext = createNewContext();
	const_cast<GMXRenderContext*>(context)->setGlxContext(glxContext);

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

	XSetWMProtocols(context->getDisplay(), getWindowHandle(), const_cast<Atom*>(&context->getAtomDeleteWindow()), True);
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

	context->switchToContext();
	XMapWindow(context->getDisplay(), getWindowHandle());
	XFree(visualInfo);

	XEvent eventReturnBuffer;
	XPeekIfEvent(context->getDisplay(), &eventReturnBuffer, &windowIsVisible_Predicator, (XPointer)getWindowHandle());
}

GLXContext GMWindow_OpenGL::createNewContext()
{
	D(d);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(getContext());
	Display* display = context->getDisplay();
	GMint renderType = GLX_RGBA_TYPE;
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

	GMint attributes[9];
	GMint where = 0;
	GMint contextFlags, contextProfile;
#if GM_DEBUG
	contextFlags = GLX_CONTEXT_DEBUG_BIT_ARB | GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#else
	contextFlags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#endif
	contextProfile = GLX_CONTEXT_CORE_PROFILE_BIT_ARB | GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
	ATTRIB_VAL(GLX_CONTEXT_FLAGS_ARB, contextFlags);
	ATTRIB_VAL(GLX_CONTEXT_PROFILE_MASK_ARB, contextProfile);
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
		GMXRenderContext* context = new GMXRenderContext(this, getenv("DISPLAY"));
		d->context.reset(context);
		context->setWindow(this);
		context->setEngine(getGraphicEngine());
	}
	return d->context.get();
}

void GMWindow_OpenGL::dispose()
{
	D(d);
	gm::GMWindowHandle wnd = getWindowHandle();
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
		setWindowHandle(0);
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
