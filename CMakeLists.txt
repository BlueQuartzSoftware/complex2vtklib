cmake_minimum_required(VERSION 3.19)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Utility.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/VtkInstall.cmake)

project(complex2VtkLib
  VERSION 0.1.0
  DESCRIPTION "Library for converting complex data structure to VTK-compatible classes."
  LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(complex2VtkLib SHARED)
add_library(complex2VtkLib::complex2VtkLib ALIAS complex2VtkLib)

# -------------------------------------------
# Find VTK
set(C2V_VtkComponents
  FiltersGeometry
  FiltersModeling
  FiltersSources
  # RenderingCore
  RenderingFreeType
  # RenderingImage
  # RenderingOpenGL2
  # RenderingVolumeOpenGL2
  # RenderingContextOpenGL2
  # RenderingAnnotation
  # InteractionWidgets
  # InteractionStyle
  IOLegacy
  IOImage
  IOGeometry
  # ChartsCore
  # ViewsContext2D
  CACHE INTERNAL "VTK Components for complex2VtkLib" FORCE
)
find_package(VTK "9.0" COMPONENTS ${C2V_VtkComponents} CONFIG REQUIRED)
AddVtkCopyInstallRules(LIBS ${C2V_VtkComponents})


option(COMPLEX_BUILD_TESTS "Enable building COMPLEX tests" ON)
enable_vcpkg_manifest_feature(TEST_VAR COMPLEX_BUILD_TESTS FEATURE "tests")

# --------------------------------------------------------------------------------------------------
# Find and include the `complex` repository. This will invoke vcpkg to ensure the dependent libraries
# are all downloaded and available
# --------------------------------------------------------------------------------------------------
get_filename_component(LIBRARY_PARENT "${PROJECT_SOURCE_DIR}" DIRECTORY CACHE)
# ------------------------------------------------------------------------------
# If the developer has set another COMPLEX directory then use that, otherwise look
# for the COMPLEX directory at the same level as the source directory
if("${COMPLEX_SOURCE_DIR}" STREQUAL "")
  if(EXISTS "${LIBRARY_PARENT}/complex")
    set(COMPLEX_SOURCE_DIR "${LIBRARY_PARENT}/complex")
  else()
    message(FATAL_ERROR "COMPLEX_SOURCE_DIR was not set. Where is the COMPLEX project directory. Please set the COMPLEX_SOURCE_DIR variable to the COMPLEX directory.")
  endif()
endif()

add_subdirectory("${COMPLEX_SOURCE_DIR}" ${PROJECT_BINARY_DIR}/complex)

# --------------------------------------------------------
# Generated headers
include(GenerateExportHeader)
set(C2V_GENERATED_DIR ${PROJECT_BINARY_DIR}/generated)
set(C2V_GENERATED_HEADER_DIR ${PROJECT_BINARY_DIR}/generated/complex2VtkLib)
set(C2V_EXPORT_HEADER ${C2V_GENERATED_HEADER_DIR}/complex2VtkLib_export.hpp)
generate_export_header(complex2VtkLib
  EXPORT_FILE_NAME ${C2V_EXPORT_HEADER}
  CUSTOM_CONTENT_FROM_VARIABLE COMPLEX_EXPORT_HEADER_EXTRA
)

# ------------------------------------------------------------------------------
# Set project source files
set(BRIDGE_DIR "${PROJECT_SOURCE_DIR}/src/complex2VtkLib/VtkBridge")

set(BRIDGE_HDRS
  ${BRIDGE_DIR}/CVArray.hpp
  ${BRIDGE_DIR}/CVEdgeGeom.hpp
  ${BRIDGE_DIR}/CVQuadGeom.hpp
  ${BRIDGE_DIR}/CVTetrahedralGeom.hpp
  ${BRIDGE_DIR}/CVTriangleGeom.hpp
  ${BRIDGE_DIR}/CVVertexGeom.hpp
  ${BRIDGE_DIR}/VtkBridge.hpp
  ${BRIDGE_DIR}/VtkMacros.hpp
)

set(BRIDGE_SRCS
  ${BRIDGE_DIR}/CVEdgeGeom.cpp
  ${BRIDGE_DIR}/CVQuadGeom.cpp
  ${BRIDGE_DIR}/CVTetrahedralGeom.cpp
  ${BRIDGE_DIR}/CVTriangleGeom.cpp
  ${BRIDGE_DIR}/CVVertexGeom.cpp
  ${BRIDGE_DIR}/VtkBridge.cpp
)

set(BRIDGE_GENERATED_HEADERS
  ${C2V_EXPORT_HEADER}
)

# ------------------------------------
# Set project settings
target_compile_features(complex2VtkLib
  PUBLIC
    cxx_std_17
)

set_target_properties(complex2VtkLib
  PROPERTIES
    DEBUG_POSTFIX "_d"
)

target_compile_options(complex2VtkLib
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

target_sources(complex2VtkLib
  PRIVATE
    ${BRIDGE_HDRS}
    ${BRIDGE_SRCS}
    ${BRIDGE_GENERATED_HEADERS}
)

source_group(TREE "${PROJECT_SOURCE_DIR}/src/complex2VtkLib" PREFIX "${PROJECT_NAME}" FILES ${BRIDGE_HDRS})
source_group(TREE "${PROJECT_SOURCE_DIR}/src/complex2VtkLib" PREFIX "${PROJECT_NAME}" FILES ${BRIDGE_SRCS})
source_group(TREE ${C2V_GENERATED_HEADER_DIR} PREFIX "generated" FILES ${BRIDGE_GENERATED_HEADERS})

set(C2V_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)
target_include_directories(complex2VtkLib
  PUBLIC
    $<BUILD_INTERFACE:${C2V_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${C2V_GENERATED_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(complex2VtkLib
  PUBLIC
    complex::complex
    ${VTK_LIBRARIES}
)
vtk_module_autoinit(TARGETS complex2VtkLib MODULES ${VTK_LIBRARIES})