#ifndef __GMGLSHADOWMAP_H__
#define __GMGLSHADOWMAP_H__
#include "common.h"
#include "gmglshaders.h"
#include "utilities/vmath.h"
BEGIN_NS

class GMGLGraphicEngine;
class GameLight;
class GMGLShadowMapping
{
public:
	struct State
	{
		vmath::mat4 lightViewMatrix;
		vmath::mat4 lightProjectionMatrix;
		GLint viewport[4]; //x, y, width, height
	};

public:
	GMGLShadowMapping(GMGLGraphicEngine&);
	~GMGLShadowMapping();

public:
	void init();
	void dispose();
	void beginDrawDepthBuffer(GameLight* shadowSourceLight);
	void endDrawDepthBuffer();
	bool hasBegun();
	GMGLShaders& getShaders();
	const State& getState();
	GMuint getDepthTexture();

private:
	GLuint m_depthTexture;
	GLuint m_frameBuffer;
	GMGLShaders m_shaders;
	GMGLGraphicEngine& m_engine;
	bool m_beginDraw;
	State m_state;
	GMuint m_shadowMapWidth;
	GMuint m_shadowMapHeight;
};

END_NS
#endif