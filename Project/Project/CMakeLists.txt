cmake_minimum_required(VERSION 2.7)

# Name of the project
project(lab1)

# Use glob to get the list of all source files.
file(GLOB SOURCES "src/*.cpp" "resources/src/*.c" "src/BaseCode/*.cpp" "resources/include/imgui-master/*.cpp" "resources/include/imgui-master/examples/imgui_impl_glfw.cpp" "resources/include/imgui-master/examples/imgui_impl_opengl3.cpp")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "src/BaseCode/*.h" "resources/include/*.h" "resources/include/glad/*.h" "resources/include/KHR/*.h" "resources/include/irrKlang/include/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

include_directories("resources/include")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})



# Add GLFW
# Get the GLFW environment variable.
# There should be a CMakeLists.txt in the specified directory.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(GLFW_DIR)
  message(STATUS "GLFW environment variable found")

  option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
  option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
  option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
  if(CMAKE_BUILD_TYPE MATCHES Release)
    add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
  else()
    add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
  endif()

  include_directories(${GLFW_DIR}/include)
  target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})
else()
  message(STATUS "GLFW environment variable `GLFW_DIR` not found, GLFW3 must be installed with the system")

  find_package(PkgConfig)
  if (PKGCONFIG_FOUND)
    message(STATUS "PkgConfig found")
    pkg_search_module(GLFW REQUIRED glfw3)
    include_directories(${GLFW_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${GLFW_LIBRARIES})
  else()
    message(STATUS "No PkgConfig found")
    find_package(glfw3 REQUIRED)
    include_directories(${GLFW_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} glfw)
  endif()
endif()



# Add GLM
# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if(GLM_INCLUDE_DIR)
  include_directories(${GLM_INCLUDE_DIR})
  message(STATUS "GLM environment variable found")
else()
# If the GLM_INCLUDE_DIR environment variable is not set, we assume
# the user has installed GLM properly on their system
  message(STATUS "GLM environment variable `GLM_INCLUDE_DIR` not found, GLM must be installed with the system")
endif()



# OS specific options and libraries
if(WIN32)
  # c++0x is enabled by default.
  # -Wall produces way too many warnings.
  # -pedantic is not supported.
  #target_link_libraries(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/resources/include/irrKlang/lib/Win32-visualStudio/irrKlang.lib)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/resources/include/irrKlang/lib/Win64/irrKlang.lib)
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)

    #Copy Dll's into binary folder

  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/irrKlang/lib/Win64/irrKlang.dll
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/irrKlang/lib/Win64/ikpMP3.dll
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/irrKlang/lib/Win64/irrKlang.dll
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/irrKlang/lib/Win64/ikpMP3.dll
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

else()


  target_link_libraries(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/resources/include/irrKlang/lib/linux/libIrrKlang.so)

  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/include/irrKlang/lib/linux/libikpMP3.so
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/libikpMP3.so ${CMAKE_CURRENT_BINARY_DIR}/ikpMP3.so)

  # Enable all pedantic warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")

  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()
endif()
