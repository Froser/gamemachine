CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT (demo)

INCLUDE_DIRECTORIES(
		../3rdparty/glew/include
		../3rdparty/glew
		../3rdparty/zlib
		../gamemachine/src
	)

LINK_DIRECTORIES(
		../3rdparty/glew/lib/release/win32
		../3rdparty/jpeg-6
		../3rdparty/freetype-2.3.5/lib
	)

LINK_LIBRARIES(
	freetype.lib
)

ADD_EXECUTABLE(demo
	main.cpp
)
