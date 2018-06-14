/*!
  此头文件声明了一些绘制相关的枚举类，它决定了在绘制每一个图元时的绘制方式。
*/

#ifndef __GMENUMS_H__
#define __GMENUMS_H__
#include <gmcommon.h>
BEGIN_NS

//! 融混函数
/*!
  在融混的时候，可以用这些参数来控制融混的效果。
*/
enum class GMS_BlendFunc
{
	ZERO = 0, //!< RGBA融混参数均为0
	ONE, //!< RGBA融混参数均为1
	SRC_COLOR, //!< RGBA融混参数分别为源的RGBA值，相当于OpenGL中的GL_SRC_COLOR
	DST_COLOR, //!< RGBA融混参数分别为目标的RGBA值，相当于OpenGL中的GL_DST_COLOR
	SRC_ALPHA, //!< RGBA融混参数都为源的Alpha值，相当于OpenGL中的GL_SRC_ALPHA
	DST_ALPHA, //!< RGBA融混参数都为目标的Alpha值，相当于OpenGL中的GL_DST_ALPHA
	ONE_MINUS_SRC_ALPHA, //!< RGBA融混参数为源的(1-Alpha)值，相当于OpenGL中的GL_ONE_MINUS_SRC_ALPHA
	ONE_MINUS_DST_ALPHA, //!< RGBA融混参数都为目标的(1-Alpha)值，相当于OpenGL中的GL_ONE_MINUS_DST_ALPHA
	ONE_MINUS_DST_COLOR, //!< RGBA融混参数分别为目标的(1,1,1,1)-(RGBA)值，相当于OpenGL中的GL_ONE_MINUS_DST_ALPHA
	MAX_OF_BLEND_FUNC,
};

//! 融混运算
/*!
  使用何种运算来计算融混结果。
*/
enum class GMS_BlendOp
{
	ADD,
	SUBSTRACT,
	REVERSE_SUBSTRACT,
	MAX_OF_BLEND_OP,
};

//! 是否进行面剔除的枚举
/*!
  通过设置GMS_Cull，可以设置一个图元在绘制的时候是否要进行面剔除。
*/
enum class GMS_Cull
{
	CULL = 0, //!< 绘制时进行面剔除
	NONE, //!< 绘制时禁用面剔除
};

//! 多边形正方向枚举
/*!
  在某些特殊的绘制场合（如面剔除），需要使用到多边形的正方向。使用左手坐标系。
*/
enum class GMS_FrontFace
{
	CLOCKWISE, //!< 窗口坐标上投影多边形的顶点顺序为顺时针方向的表面为正面
	COUNTER_CLOCKWISE, //!< 窗口坐标上投影多边形的顶点顺序为逆时针方向的表面为正面
};

//! 纹理过滤方式
/*!
  根据设置纹理过滤方式，来决定纹理的绘制效果，指定纹理在缩放、放大时使用不同的过滤算法。
*/
enum class GMS_TextureFilter
{
	LINEAR = 0, //!< 双线性过滤，相当于OpenGL的GL_LINEAR
	NEAREST, //!< 最近邻过滤，相当于OpenGL的GL_NEAREST
	LINEAR_MIPMAP_LINEAR, //!< 三线性过滤（使用MIP贴图级别之间插值的双线性过滤），相当于OpenGL的GL_LINEAR_MIPMAP_LINEAR
	NEAREST_MIPMAP_LINEAR, //!< 使用MIP贴图级别之间插值的最近邻过滤，相当于OpenGL的GL_NEAREST_MIPMAP_LINEAR
	LINEAR_MIPMAP_NEAREST, //!< 使用MIP贴图的双线性过滤，相当于OpenGL的GL_LINEAR_MIPMAP_NEAREST
	NEAREST_MIPMAP_NEAREST, //!< 使用MIP贴图的最近邻过滤，相当于OpenGL的GL_NEAREST_MIPMAP_NEAREST
};

//! 纹理的环绕方式
/*!
  纹理的环绕方式可以决定纹理坐标超出坐标范围时的行为。
*/
enum class GMS_Wrap
{
	REPEAT = 0, //!< 重复环绕，相当于忽略掉纹理坐标的整数部分，相当于GL_REPEAT
	CLAMP_TO_EDGE, //!< 超出纹理坐标的纹理取纹理的边的像素，相当于GL_CLAMP_TO_EDGE
	CLAMP_TO_BORDER, //!< 超出纹理坐标的纹理取纹理的角的像素，相当于GL_CLAMP_TO_BORDER
	MIRRORED_REPEAT, //!< 镜像环绕，相当于将纹理坐标1.1变成0.9，相当于GL_MIRRORED_REPEAT
};

//! 纹理变换方式
/*!
  指定纹理变换方式，可以在绘制时对纹理进行仿射变换。
  \sa GMS_TextureMod
*/
enum class GMS_TextureModType
{
	NO_TEXTURE_MOD = 0, //!< 不使用纹理变换
	SCROLL, //!< 纹理在绘制时发生位移
	SCALE, //!< 纹理在绘制时发生拉伸
};

END_NS
#endif