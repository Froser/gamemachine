#ifndef __GAMEMACHINEQT_FORWARD_H__
#define __GAMEMACHINEQT_FORWARD_H__

#ifdef GM_QT_DLL
#	ifndef GM_QT_EXPORT
#		define GM_QT_EXPORT GM_DECL_EXPORT
#	endif
#else
#	if GM_USE_QT_DLL
#		ifndef GM_QT_EXPORT
#			define GM_QT_EXPORT GM_DECL_IMPORT
#		endif
#	else
#		ifndef GM_QT_EXPORT
#			define GM_QT_EXPORT
#		endif
#	endif
#endif

// Fwds
namespace gm
{
	class GMGameWorld;
	class GMGameObject;
	struct IGameHandler;
	struct IRenderContext;
	struct GMGameMachineDesc;
}

#endif