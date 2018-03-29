#include "stdafx.h"
#include "gmmcursor.h"

bool GMUICursor::createCursor(const gm::GMCursorDesc& cursorDesc, const gm::GMImage& cursorImg)
{
	D(d);
	gm::GMint cursorWidth = GetSystemMetrics(SM_CXCURSOR);
	gm::GMint cursorHeight = GetSystemMetrics(SM_CYCURSOR);
	if (cursorImg.getWidth() != cursorWidth || cursorImg.getHeight() != cursorHeight)
		return false;

	HBITMAP hBitmap = createBitmapFromImage(cursorImg);
	d->cursor = createCursorFromBitmap(
		hBitmap,
		RGB(cursorDesc.transparentColor[0], cursorDesc.transparentColor[1], cursorDesc.transparentColor[2]),
		cursorDesc.xHotspot,
		cursorDesc.yHotspot
	);
	::DeleteObject(hBitmap);

	return !!d->cursor;
}

bool GMUICursor::setCursor(gm::GMWindowHandle handle)
{
	D(d);
	HCURSOR cursor = SetCursor(d->cursor);
	return !!cursor;
}

GMUICursor::~GMUICursor()
{
	D(d);
	DestroyCursor(d->cursor);
}

HBITMAP GMUICursor::createBitmapFromImage(const gm::GMImage& cursorImg)
{
	UINT uWidth = cursorImg.getWidth();
	UINT uHeight = cursorImg.getHeight();
	HDC hDC = ::GetDC(NULL);
	HDC memDC = ::CreateCompatibleDC(hDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(
		hDC,
		cursorImg.getWidth(),
		cursorImg.getHeight()
	);

	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(memDC, hBitmap);
	UINT bytesPerPixel = (cursorImg.getData().format == gm::GMImageFormat::RGB ? 3 : 4);

	for (gm::GMuint y = 0; y < uHeight; ++y)
	{
		for (gm::GMuint x = 0; x < uWidth; ++x)
		{
			::SetPixel(
				memDC,
				x,
				y,
				bytesPerPixel == 3 ? RGB(
					cursorImg.getData().mip[0].data[y * uWidth + x * 3],
					cursorImg.getData().mip[0].data[y * uWidth + x * 3 + 1],
					cursorImg.getData().mip[0].data[y * uWidth + x * 3 + 2]
					) : RGB(
						cursorImg.getData().mip[0].data[y * uWidth + x * 4],
						cursorImg.getData().mip[0].data[y * uWidth + x * 4 + 1],
						cursorImg.getData().mip[0].data[y * uWidth + x * 4 + 2]
					)
			);
		}
	}

	::SelectObject(memDC, hOldBitmap);
	::ReleaseDC(NULL, hDC);
	::DeleteDC(memDC);
	return hBitmap;
}

void GMUICursor::getMaskBitmaps(HBITMAP hSourceBitmap, COLORREF clrTransparent,
	HBITMAP &hAndMaskBitmap, HBITMAP &hXorMaskBitmap)
{
	HDC hDC = ::GetDC(NULL);
	HDC hMainDC = ::CreateCompatibleDC(hDC);
	HDC hAndMaskDC = ::CreateCompatibleDC(hDC);
	HDC hXorMaskDC = ::CreateCompatibleDC(hDC);

	BITMAP bm;
	::GetObject(hSourceBitmap, sizeof(BITMAP), &bm);

	hAndMaskBitmap = ::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
	hXorMaskBitmap = ::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);

	HBITMAP hOldMainBitmap = (HBITMAP)::SelectObject(hMainDC, hSourceBitmap);
	HBITMAP hOldAndMaskBitmap = (HBITMAP)::SelectObject(hAndMaskDC, hAndMaskBitmap);
	HBITMAP hOldXorMaskBitmap = (HBITMAP)::SelectObject(hXorMaskDC, hXorMaskBitmap);

	COLORREF cMainBitPixel;
	for (int y = 0; y < bm.bmHeight; ++y)
	{
		for (int x = 0; x < bm.bmWidth; ++x)
		{
			cMainBitPixel = ::GetPixel(hMainDC, x, y);
			if (cMainBitPixel == clrTransparent)
			{
				::SetPixel(hAndMaskDC, x, y, RGB(255, 255, 255));
				::SetPixel(hXorMaskDC, x, y, RGB(0, 0, 0));
			}
			else
			{
				::SetPixel(hAndMaskDC, x, y, RGB(0, 0, 0));
				::SetPixel(hXorMaskDC, x, y, cMainBitPixel);
			}
		}
	}

	::SelectObject(hMainDC, hOldMainBitmap);
	::SelectObject(hAndMaskDC, hOldAndMaskBitmap);
	::SelectObject(hXorMaskDC, hOldXorMaskBitmap);

	::DeleteDC(hXorMaskDC);
	::DeleteDC(hAndMaskDC);
	::DeleteDC(hMainDC);

	::ReleaseDC(NULL, hDC);
}

HCURSOR GMUICursor::createCursorFromBitmap(
	HBITMAP hSourceBitmap,
	COLORREF clrTransparent,
	DWORD xHotspot,
	DWORD yHotspot
)
{
	HCURSOR hRetCursor = NULL;

	do
	{
		if (NULL == hSourceBitmap)
		{
			break;
		}

		HBITMAP hAndMask = NULL;
		HBITMAP hXorMask = NULL;
		getMaskBitmaps(hSourceBitmap, clrTransparent, hAndMask, hXorMask);
		if (NULL == hAndMask || NULL == hXorMask)
		{
			break;
		}

		ICONINFO iconinfo = { 0 };
		iconinfo.fIcon = FALSE;
		iconinfo.xHotspot = xHotspot;
		iconinfo.yHotspot = yHotspot;
		iconinfo.hbmMask = hAndMask;
		iconinfo.hbmColor = hXorMask;

		hRetCursor = ::CreateIconIndirect(&iconinfo);

	} while (0);

	return hRetCursor;
}