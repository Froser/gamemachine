#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "gmxrendercontext.h"
#include "gmgl/gmglgraphic_engine.h"

#define ATTRIB(a) attributes[where++]=(a)
#define ATTRIB_VAL(a, v) { ATTRIB(a); ATTRIB(v); }
#define MAX_ATTRIBS 100
#define ATTRIBS attributes
#define INIT_ATTRIBS() GMint attributes[MAX_ATTRIBS]; GMint where = 0;

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

public:
	virtual IGraphicEngine* getGraphicEngine() override;
	virtual const IRenderContext* getContext() override;
	virtual void onWindowCreated(const GMWindowAttributes& wndAttrs) override;

private:
	GLXContext createNewContext();
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
	setWindowHandle(window);

	// CreateContext
	GLXContext glxContext = createNewContext();
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

	GMint attributes[9];
	// TODO FillAttributes
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
		GMXRenderContext* context = new GMXRenderContext(getenv("DISPLAY"));
		d->context.reset(context);
		context->setWindow(this);
		context->setEngine(getGraphicEngine());
	}
	return d->context.get();
}

bool GMWindowFactory::createWindowWithOpenGL(GMInstance instance, IWindow* parent, OUT IWindow** window)
{
	// (*window) = new GMWindow_OpenGL(parent);
	// if (*window)
	//	return true;
	return false;
}
