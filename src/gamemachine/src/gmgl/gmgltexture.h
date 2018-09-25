#ifndef __GMGL_TEXTURE_H__
#define __GMGL_TEXTURE_H__
#include <gmcommon.h>
#include <gmimage.h>
#include "foundation/utilities/tools.h"
#include "gmdata/gmmodel.h"
#include <map>
#include <string>
#include "shader_constants.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLTexture)
{
	bool inited = false;
	GMuint32 id = 0;
	GMuint32 target = 0;
	GMuint32 format = 0;
	GMuint32 internalFormat = 0;
	GMuint32 dataType = 0;
	const GMImage* image = nullptr;
	bool texParamsSet = false;
};

class GMGLTexture : public ITexture
{
	GM_DECLARE_PRIVATE_NGO(GMGLTexture)

public:
	GMGLTexture(const GMImage* image);
	~GMGLTexture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;
};

GM_PRIVATE_OBJECT(GMGLWhiteTexture)
{
	GMuint32 textureId = 0;
	const IRenderContext* context = nullptr;
};

class GMGLWhiteTexture : public ITexture
{
	GM_DECLARE_PRIVATE(GMGLWhiteTexture)

public:
	GMGLWhiteTexture(const IRenderContext* context);
	~GMGLWhiteTexture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;
};

END_NS
#endif