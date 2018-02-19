set(TARGET_NAME ViewTransformer)
add_library(${TARGET_NAME} STATIC "src/${TARGET_NAME}.cpp;")
target_include_directories(${TARGET_NAME} PUBLIC "incl")

#set(FILE ViewTransformer)
#get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
#target_include_directories(TEST PUBLIC "${PARENT_DIR}/incl")
#set(SOURCE_FILES "${SOURCE_FILES}${PARENT_DIR}/src/${FILE}.cpp;" PARENT_SCOPE)
