# CMakeList.txt : CMake project for LearnOpenGL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (LearnOpenGL LANGUAGES C CXX)

# Allow overriding include/lib dirs from the CLI or GUI
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/includes" CACHE PATH "Project include directory")
set(PROJECT_LIB_DIR     "${PROJECT_SOURCE_DIR}/lib"      CACHE PATH "Project lib directory")

# Add source to this project's executable.
add_executable (LearnOpenGL "${PROJECT_SOURCE_DIR}/src/main.cpp")

# If a local glad.c exists add it to the target sources so you don't need a separate target
if (EXISTS "${PROJECT_SOURCE_DIR}/src/glad.c")
  target_sources(LearnOpenGL PRIVATE "${PROJECT_SOURCE_DIR}/src/glad.c")
  message(STATUS "Added local src/glad.c to LearnOpenGL target")
endif()

# If a local stb implementation file exists (or we create one) add it
if (EXISTS "${PROJECT_SOURCE_DIR}/src/stb_image.cpp")
  target_sources(LearnOpenGL PRIVATE "${PROJECT_SOURCE_DIR}/src/stb_image.cpp")
  message(STATUS "Added local src/stb_image.cpp (STB_IMAGE_IMPLEMENTATION) to LearnOpenGL target")
endif()

# Add include folder to the target's include path
target_include_directories(LearnOpenGL PRIVATE "${PROJECT_INCLUDE_DIR}")

# Helper: create an IMPORTED target if a lib file exists in PROJECT_LIB_DIR
function(create_imported_lib target_name lib_path include_dir)
  if (EXISTS "${lib_path}")
    add_library(${target_name} STATIC IMPORTED)
    set_target_properties(${target_name} PROPERTIES
      IMPORTED_LOCATION "${lib_path}"
      INTERFACE_INCLUDE_DIRECTORIES "${include_dir}"
    )
    message(STATUS "Created imported target ${target_name} -> ${lib_path}")
    return()
  endif()
endfunction()

# 1) Try find_package for GLFW (preferred when using vcpkg or package config)
find_package(glfw3 CONFIG QUIET)
if (TARGET glfw)
  message(STATUS "Using GLFW target provided by package: glfw")
  target_link_libraries(LearnOpenGL PRIVATE glfw)
elseif (TARGET GLFW::GLFW)
  message(STATUS "Using GLFW::GLFW imported target")
  target_link_libraries(LearnOpenGL PRIVATE GLFW::GLFW)
else()
  # 2) Try prebuilt libs in PROJECT_LIB_DIR (look for common filenames)
  if (EXISTS "${PROJECT_LIB_DIR}/glfw3.lib")
    create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/glfw3.lib" "${PROJECT_INCLUDE_DIR}")
    target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
  elseif (EXISTS "${PROJECT_LIB_DIR}/glfw3dll.lib")
    create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/glfw3dll.lib" "${PROJECT_INCLUDE_DIR}")
    target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
  else()
    message(WARNING "GLFW not found via find_package and no glfw3.lib/glfw3dll.lib in ${PROJECT_LIB_DIR}. "
                    "Either install GLFW via vcpkg or place the correct .lib (matching architecture) into ${PROJECT_LIB_DIR}.")
  endif()
endif()

# GLAD: either compile the local source (glad.c), or link a prebuilt lib if present
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/src/glad.c")
  if (EXISTS "${PROJECT_LIB_DIR}/glad.lib")
    create_imported_lib(Glad_prebuilt "${PROJECT_LIB_DIR}/glad.lib" "${PROJECT_INCLUDE_DIR}")
    target_link_libraries(LearnOpenGL PRIVATE Glad_prebuilt)
  endif()
else()
  # glad.c already added above to sources if present
  message(STATUS "Using local glad.c implementation")
endif()

# STB_IMAGE: prefer compiling the implementation unit into the target (we added src/stb_image.cpp).
# If you have a prebuilt lib instead, link it similarly.
if (EXISTS "${PROJECT_LIB_DIR}/stb_image.lib")
  create_imported_lib(StbImage_prebuilt "${PROJECT_LIB_DIR}/stb_image.lib" "${PROJECT_INCLUDE_DIR}")
  target_link_libraries(LearnOpenGL PRIVATE StbImage_prebuilt)
else()
  message(STATUS "No prebuilt stb_image.lib found; ensure src/stb_image.cpp exists (defines STB_IMAGE_IMPLEMENTATION) or add library to ${PROJECT_LIB_DIR}")
endif()

# OpenGL (platform-provided) -- prefer target if available
find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
  target_link_libraries(LearnOpenGL PRIVATE OpenGL::GL)
endif()

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# Set C++ standard for the target
set_target_properties(LearnOpenGL PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)

# Helpful status output
message(STATUS "PROJECT_INCLUDE_DIR = ${PROJECT_INCLUDE_DIR}")
message(STATUS "PROJECT_LIB_DIR     = ${PROJECT_LIB_DIR}")