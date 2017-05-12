project (libmad)

include_directories(
		./
	)

set(SOURCES
		bit.h
		decoder.h
		fixed.h
		frame.h
		global.h
		huffman.h
		layer12.h
		layer3.h
		mad.h
		stream.h
		synth.h
		timer.h
		version.h
		bit.c
		decoder.c
		fixed.c
		frame.c
		huffman.c
		layer12.c
		layer3.c
		stream.c
		synth.c
		timer.c
		version.c
	)

add_library(libmad
		${SOURCES}
	)

add_definitions(
	-DFPM_DEFAULT
	-D_LIB
	-DHAVE_CONFIG_H
	-DASO_ZEROCHECK
)

set_target_properties(libmad PROPERTIES FOLDER ${PROJECT_NAME})