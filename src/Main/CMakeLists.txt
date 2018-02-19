set(FILE opencvtest)
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(SOURCE_FILES "${SOURCE_FILES}${PARENT_DIR}/src/${FILE}.cpp;" PARENT_SCOPE)
