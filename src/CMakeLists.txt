cmake_minimum_required(VERSION 2.8)
project(carolocup)
set(LIBRARIES "ViewTransformer;")

# Should find all OpenCV libs
find_package(OpenCV REQUIRED)

if(${DEBUG})
   # If the package has been found, several variables will
   # be set, you can find the full list with descriptions
   # in the OpenCVConfig.cmake file.
   # Print some message showing some of them
   message(STATUS "OpenCV library status:")
   message(STATUS "    version: ${OpenCV_VERSION}")
   message(STATUS "    libraries: ${OpenCV_LIBS}")
   message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
   message(STATUS "    path: ${OpenCV_DIR}")
endif()

if(CMAKE_VERSION VERSION_LESS "2.8.11")
    # Add OpenCV headers location to your include paths
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# TODO: some more linker flag might be required here
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(${LIBRARIES})
add_subdirectory(Main)

message(STATUS ${SOURCE_FILES})


add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
target_link_libraries (${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

