set(TARGET_NAME Serial)
add_library(${TARGET_NAME} STATIC "src/${TARGET_NAME}.cpp;")
target_include_directories(${TARGET_NAME} PUBLIC "incl")
