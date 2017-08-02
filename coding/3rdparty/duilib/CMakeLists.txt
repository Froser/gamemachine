project(duilib)

set (SOURCES
		StdAfx.cpp
		StdAfx.h
		UIlib.cpp
		UIlib.h

		Core/UIBase.cpp
		Core/UIBase.h
		Core/UIContainer.cpp
		Core/UIContainer.h
		Core/UIControl.cpp
		Core/UIControl.h
		Core/UIDefine.h
		Core/UIDlgBuilder.cpp
		Core/UIDlgBuilder.h
		Core/UIManager.cpp
		Core/UIManager.h
		Core/UIMarkup.cpp
		Core/UIMarkup.h
		Core/UIRender.cpp
		Core/UIRender.h

		Layout/UIChildLayout.cpp
		Layout/UIChildLayout.h
		Layout/UIHorizontalLayout.cpp
		Layout/UIHorizontalLayout.h
		Layout/UITabLayout.cpp
		Layout/UITabLayout.h
		Layout/UITileLayout.cpp
		Layout/UITileLayout.h
		Layout/UIVerticalLayout.cpp
		Layout/UIVerticalLayout.h

		Control/UIActiveX.cpp
		Control/UIActiveX.h
		Control/UIButton.cpp
		Control/UIButton.h
		Control/UICheckBox.cpp
		Control/UICheckBox.h
		Control/UICombo.cpp
		Control/UICombo.h
		Control/UIDateTime.cpp
		Control/UIDateTime.h
		Control/UIEdit.cpp
		Control/UIEdit.h
		Control/UIFlash.cpp
		Control/UIFlash.h
		Control/UIGifAnim.cpp
		Control/UIGifAnim.h
		Control/UILabel.cpp
		Control/UILabel.h
		Control/UIList.cpp
		Control/UIList.h
		Control/UIOption.cpp
		Control/UIOption.h
		Control/UIProgress.cpp
		Control/UIProgress.h
		Control/UIRichEdit.cpp
		Control/UIRichEdit.h
		Control/UIScrollBar.cpp
		Control/UIScrollBar.h
		Control/UISlider.cpp
		Control/UISlider.h
		Control/UIText.cpp
		Control/UIText.h
		Control/UITreeView.cpp
		Control/UITreeView.h
		Control/UIWebBrowser.cpp
		Control/UIWebBrowser.h

		Utils/downloadmgr.h
		Utils/flash11.tlh
		Utils/FlashEventHandler.h
		Utils/stb_image.c
		Utils/stb_image.h
		Utils/UIDelegate.cpp
		Utils/UIDelegate.h
		Utils/Utils.cpp
		Utils/Utils.h
		Utils/WebBrowserEventHandler.h
		Utils/WinImplBase.cpp
		Utils/WinImplBase.h
		Utils/WndShadow.cpp
		Utils/WndShadow.h
		Utils/XUnzip.cpp
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Layout)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)
add_definitions(-DUNICODE -D_UNICODE)

source_group_by_dir(SOURCES)
add_library(
		duilib ${SOURCES}
	)

set_target_properties(duilib PROPERTIES FOLDER ${PROJECT_NAME})

target_link_libraries(duilib comctl32)