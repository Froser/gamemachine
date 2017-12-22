project(gamemachinedocs)
set (GM_DOCS_SOURCES ${gamemachine_SOURCE_DIR}/src)
set (GM_DOCS_OUTPUT ${gamemachinedocs_BINARY_DIR})
configure_file(Doxyfile.in Doxyfile @ONLY)
add_custom_target(gamemachinedocs ALL "${DOXYGEN_EXECUTABLE}"
                  WORKING_DIRECTORY "./"
                  COMMENT "Generating HTML documentation" VERBATIM)