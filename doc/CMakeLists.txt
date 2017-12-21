configure_file(Doxyfile.in Doxyfile @ONLY)
add_custom_target(gamemachinedocs ALL "${DOXYGEN_EXECUTABLE}"
                  WORKING_DIRECTORY "./"
                  COMMENT "Generating HTML documentation" VERBATIM)