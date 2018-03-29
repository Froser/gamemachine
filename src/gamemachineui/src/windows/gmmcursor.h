#ifndef __GMUICURSOR_H__
#define __GMUICURSOR_H__
#include <gmui.h>
#include <defines.h>
#include <gamemachine.h>

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUICursor)
{
	HCURSOR cursor = NULL;
};

class GMUICursor : public gm::ICursor
{
	DECLARE_PRIVATE(GMUICursor)

public:
	GMUICursor() = default;
	~GMUICursor();

public:
	virtual bool createCursor(const gm::GMCursorDesc& cursorDesc, const gm::GMImage& cursorImg) override;
	virtual bool setCursor(gm::GMWindowHandle handle) override;

	virtual bool isEmpty() override
	{
		D(d);
		return !d->cursor;
	}

private:
	HBITMAP createBitmapFromImage(
		const gm::GMImage& cursorImg
	);

	void getMaskBitmaps(
		HBITMAP hSourceBitmap,
		COLORREF clrTransparent,
		HBITMAP &hAndMaskBitmap,
		HBITMAP &hXorMaskBitmap
	);

	HCURSOR createCursorFromBitmap(
		HBITMAP hSourceBitmap,
		COLORREF clrTransparent,
		DWORD xHotspot,
		DWORD yHotspot
	);
};

END_UI_NS
#endif