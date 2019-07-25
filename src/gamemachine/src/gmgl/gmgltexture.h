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

GM_PRIVATE_CLASS(GMGLTexture);
class GMGLTexture : public ITexture
{
	GM_DECLARE_PRIVATE(GMGLTexture)
	GM_DISABLE_COPY_ASSIGN(GMGLTexture)

public:
	GMGLTexture(const GMImage* image);
	~GMGLTexture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;
};

GM_PRIVATE_CLASS(GMGLWhiteTexture);
class GMGLWhiteTexture : public ITexture
{
	GM_DECLARE_PRIVATE(GMGLWhiteTexture)
	GM_DISABLE_COPY_ASSIGN(GMGLWhiteTexture)

public:
	GMGLWhiteTexture(const IRenderContext* context);
	~GMGLWhiteTexture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;
};

class GMGLEmptyTexture : public GMGLWhiteTexture
{
public:
	using GMGLWhiteTexture::GMGLWhiteTexture;
	virtual void init() override;
};

END_NS
#endif