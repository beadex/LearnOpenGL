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

option(USE_GLES "Build for OpenGL ES (Snapdragon/Mobile)" OFF)

if(USE_GLES)
    add_definitions(-DPLATFORM_ARM64 -DUSE_GLES)
    message(STATUS "Configuration: OpenGL ES 3.2")
else()
    message(STATUS "Configuration: OpenGL 3.3 Desktop")
endif()

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

# Helper: create an IMPORTED shared target for DLL/.lib pairs (works if you copied only the .dll as well)
function(create_imported_shared target_name dll_path lib_path include_dir)
  if (EXISTS "${dll_path}" OR EXISTS "${lib_path}")
    add_library(${target_name} SHARED IMPORTED)
    set_property(TARGET ${target_name} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${include_dir}")
    if (EXISTS "${lib_path}")
      # import library available (preferred for linking on MSVC)
      set_target_properties(${target_name} PROPERTIES
        IMPORTED_IMPLIB "${lib_path}"
        IMPORTED_LOCATION "${dll_path}"
      )
      message(STATUS "Created imported shared target ${target_name} -> implib:${lib_path} dll:${dll_path}")
    else()
      # only DLL present — set IMPORTED_LOCATION so runtime copy is known; linking may still require an import lib on MSVC
      set_target_properties(${target_name} PROPERTIES
        IMPORTED_LOCATION "${dll_path}"
      )
      message(STATUS "Created imported shared target ${target_name} -> dll:${dll_path} (no import lib found)")
    endif()
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
  if(USE_GLES)
    if (UNIX AND NOT APPLE)
        if (EXISTS "${PROJECT_LIB_DIR}/arm64/libglfw3.a")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/arm64/libglfw3.a" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        else()
            message(WARNING "GLFW not found via find_package and no arm64/libglfw3.a in ${PROJECT_LIB_DIR}. "
                    "Either install GLFW via vcpkg or place the correct .a (matching architecture) into ${PROJECT_LIB_DIR}.")
        endif()
    elseif (WIN32)
        if (EXISTS "${PROJECT_LIB_DIR}/arm64/glfw3.lib")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/arm64/glfw3.lib" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        elseif (EXISTS "${PROJECT_LIB_DIR}/arm64/glfw3dll.lib")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/arm64/glfw3dll.lib" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        else()
            message(WARNING "GLFW not found via find_package and no arm64/glfw3.lib/glfw3dll.lib in ${PROJECT_LIB_DIR}. "
                    "Either install GLFW via vcpkg or place the correct .lib (matching architecture) into ${PROJECT_LIB_DIR}.")
        endif()
    else()
        message(WARNING "Unsupported platform")
    endif()
  else()
    if (UNIX AND NOT APPLE)
        if (EXISTS "${PROJECT_LIB_DIR}/x64/libglfw3.a")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/x64/libglfw3.a" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        else()
            message(WARNING "GLFW not found via find_package and no x64/libglfw3.a in ${PROJECT_LIB_DIR}. "
                    "Either install GLFW via vcpkg or place the correct .a (matching architecture) into ${PROJECT_LIB_DIR}.")
        endif()
    elseif (WIN32)
        if (EXISTS "${PROJECT_LIB_DIR}/x64/glfw3.lib")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/x64/glfw3.lib" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        elseif (EXISTS "${PROJECT_LIB_DIR}/x64/glfw3dll.lib")
            create_imported_lib(GLFW3_prebuilt "${PROJECT_LIB_DIR}/x64/glfw3dll.lib" "${PROJECT_INCLUDE_DIR}")
            target_link_libraries(LearnOpenGL PRIVATE GLFW3_prebuilt)
        else()
            message(WARNING "GLFW not found via find_package and no x64/glfw3.lib/glfw3dll.lib in ${PROJECT_LIB_DIR}. "
                    "Either install GLFW via vcpkg or place the correct .lib (matching architecture) into ${PROJECT_LIB_DIR}.")
        endif()
    else()
        message(WARNING "Unsupported platform")
    endif()
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

# Assimp: prefer import library (.lib) if present, otherwise allow using a .dll in the lib dir.
# Place headers under ${PROJECT_INCLUDE_DIR}/assimp and the .dll/.lib into ${PROJECT_LIB_DIR}
set(ASSIMP_DLL "${PROJECT_LIB_DIR}/assimp.dll")
set(ASSIMP_LIB "${PROJECT_LIB_DIR}/assimp.lib")
set(ASSIMP_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/assimp")

if (EXISTS "${ASSIMP_LIB}")
  create_imported_shared(Assimp_prebuilt "${ASSIMP_DLL}" "${ASSIMP_LIB}" "${ASSIMP_INCLUDE_DIR}")
  target_link_libraries(LearnOpenGL PRIVATE Assimp_prebuilt)
elseif (EXISTS "${ASSIMP_DLL}")
  create_imported_shared(Assimp_prebuilt "${ASSIMP_DLL}" "" "${ASSIMP_INCLUDE_DIR}")
  target_link_libraries(LearnOpenGL PRIVATE Assimp_prebuilt)
else()
  message(STATUS "Assimp not found in ${PROJECT_LIB_DIR}. Place assimp.dll and/or assimp.lib in ${PROJECT_LIB_DIR} and headers in ${PROJECT_INCLUDE_DIR}/assimp")
endif()

# ImGui: if checked-out as submodule under includes/imgui, add sources and includes
set(IMGUI_DIR "${PROJECT_INCLUDE_DIR}/imgui")
if (EXISTS "${IMGUI_DIR}/imgui.h")
  message(STATUS "Found ImGui in ${IMGUI_DIR} - adding sources and include directories")
  # make imgui headers visible as #include "imgui.h"
  target_include_directories(LearnOpenGL PRIVATE "${IMGUI_DIR}")

  # Core ImGui sources (required)
  target_sources(LearnOpenGL PRIVATE
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
  )
  # optional demo file
  if (EXISTS "${IMGUI_DIR}/imgui_demo.cpp")
    target_sources(LearnOpenGL PRIVATE "${IMGUI_DIR}/imgui_demo.cpp")
  endif()

  # Backends: common ones are in backends/ (GLFW + OpenGL3)
  set(IMGUI_BACKENDS "${IMGUI_DIR}/backends")
  if (EXISTS "${IMGUI_BACKENDS}/imgui_impl_glfw.cpp" AND EXISTS "${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp")
  message(STATUS "Found ImGui impl glfw and opengl3 in ${IMGUI_BACKENDS} - adding sources and include directories")
    target_sources(LearnOpenGL PRIVATE
      "${IMGUI_BACKENDS}/imgui_impl_glfw.cpp"
      "${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp"
    )
    # allow backend headers to find imgui.h via IMGUI_DIR
    target_include_directories(LearnOpenGL PRIVATE "${IMGUI_BACKENDS}")
  endif()

  # If you're compiling glad.c locally, tell ImGui backend to use glad
  if (EXISTS "${PROJECT_SOURCE_DIR}/src/glad.c")
    target_compile_definitions(LearnOpenGL PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
  endif()
else()
  message(STATUS "ImGui not found in ${IMGUI_DIR} (skipping adding ImGui sources)")
endif()

# OpenGL (platform-provided) -- prefer target if available
if(USE_GLES)
    find_path(GLES3_INCLUDE_DIR GLES3/gl32.h)
    find_path(EGL_INCLUDE_DIR EGL/egl.h)

    find_library(EGL_LIBRARY NAMES libEGL EGL)
    find_library(GLES3_LIBRARY NAMES libGLESv2 GLESv2)

    if(EGL_LIBRARY AND GLES3_LIBRARY)
        message(STATUS "Found GLES library: ${GLES3_LIBRARY} & FOUND EGL library: ${EGL_LIBRARY}")
        target_link_libraries(LearnOpenGL PRIVATE ${EGL_LIBRARY} ${GLES3_LIBRARY})
        target_include_directories(LearnOpenGL PRIVATE ${EGL_INCLUDE_DIR} ${GLES3_INCLUDE_DIR})
    else()
        message(WARNING "GLES library not found, falling back to opengl32")
        target_link_libraries(LearnOpenGL PRIVATE opengl32)
    endif()
else()
    # OpenGL for x64 Desktop
    find_package(OpenGL REQUIRED)
    if (OpenGL_FOUND)
        target_link_libraries(LearnOpenGL PRIVATE OpenGL::GL)
    endif()
endif()

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

# Set C++ standard for the target
set_target_properties(LearnOpenGL PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)

# Helpful status output
message(STATUS "PROJECT_INCLUDE_DIR = ${PROJECT_INCLUDE_DIR}")
message(STATUS "PROJECT_LIB_DIR     = ${PROJECT_LIB_DIR}")