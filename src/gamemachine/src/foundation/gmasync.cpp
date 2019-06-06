#include "stdafx.h"
#include "gmasync.h"

GMMultithreadRenderHelper::GMMultithreadRenderHelper(IWindow* window)
	: m_window(window)
{
	m_window->setMultithreadRenderingFlag(GMMultithreadRenderingFlag::StartRenderOnMultiThread);
}

GMMultithreadRenderHelper::~GMMultithreadRenderHelper()
{
	m_window->setMultithreadRenderingFlag(GMMultithreadRenderingFlag::EndRenderOnMultiThread);
}
