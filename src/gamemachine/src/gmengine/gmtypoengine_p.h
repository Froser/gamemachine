#ifndef __GM_TYPOENGINE_P_H__
#define __GM_TYPOENGINE_P_H__
#include <gmcommon.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMTypoEngine)
{
	GMFontHandle font = 0;

	GMTypoStateMachine* stateMachine = nullptr;
	const IRenderContext* context = nullptr;
	bool insetStateMachine = false;

	GMGlyphManager* glyphManager = nullptr;
	std::wstring literature;
	GMTypoOptions options;
	GMfloat lineHeight = 0;

	// 绘制状态
	GMint32 current_x = 0;
	GMint32 current_y = 0;
	GMint32 currentLineNo = 1;
	GMFontSizePt fontSize = 12;
	GMfloat color[3] = { 1.f, 1.f, 1.f };

	Vector<GMTypoResult> results;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMTypoTextBuffer)
{
	ITypoEngine* engine = nullptr;
	GMString buffer;
	GMRect rc;
	bool newline = true;
	bool dirty = false;
	GMsize_t renderStart = 0;
	GMsize_t renderEnd = 0;
};

END_NS
#endif