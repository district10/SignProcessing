project( SignProcessing2 )
cmake_minimum_required( VERSION 2.8.3 )

set( CMAKE_COLOR_MAKEFILE ON )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
add_definitions( -DUNICODE )

# Allow advanced users to generate Makefiles printing detailed commands
mark_as_advanced( CMAKE_VERBOSE_MAKEFILE )

# Path to additional CMake modules
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH} )
set( CMAKE_DEBUG_POSTFIX "d" )

# large address aware option setting
if( ${MINGW} )
    set( LARGEADDRESSAWARE "--large-address-aware" )
else( ${MINGW} )
    if ( WIN32 )
        set( LARGEADDRESSAWARE "/LARGEADDRESSAWARE" )
    else ( WIN32 )
        set( LARGEADDRESSAWARE " " )
    endif ( WIN32 )
endif( ${MINGW} )

if ( WIN32)
    SET( APP_OPT "WIN32" )
else ( WIN32 )
    if ( APPLE )
        #SET( APP_OPT "MACOSX_BUNDLE" )
        SET( APP_OPT "EXCLUDE_FROM_ALL" )
    else ( APPLE )
        SET( APP_OPT " " )
    endif ( APPLE )
endif ( WIN32 )

# OpenCV
include_directories( ${OPENCV_INCLUDE_DIR} )
include( $ENV{OpenCV3_DIR}/OpenCVConfig.cmake )
message( STATUS "OpenCV library status:" )
message( STATUS "    version: ${OpenCV_VERSION}" )
message( STATUS "    libraries: ${OpenCV_LIBS}" )
message( STATUS "    include path: ${OpenCV_INCLUDE_DIRS}" )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Qt4
find_package( Qt4 REQUIRED )
include( ${QT_USE_FILE} )

# include them all
include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/src )
include_directories( ${CMAKE_BINARY_DIR}/src )
include_directories( ${CMAKE_BINARY_DIR}/src/MLP )
include_directories( ${CMAKE_SOURCE_DIR}/src/MLP )
include_directories( ${CMAKE_BINARY_DIR}/src/SignClassifier )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignClassifier )
include_directories( ${CMAKE_BINARY_DIR}/src/SignCutter )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignCutter )
include_directories( ${CMAKE_BINARY_DIR}/src/SignLogger )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignLogger )
include_directories( ${CMAKE_BINARY_DIR}/src/SignProcessor )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignProcessor )
include_directories( ${CMAKE_BINARY_DIR}/src/SignTransformer )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignTransformer )
include_directories( ${CMAKE_BINARY_DIR}/src/Utils )
include_directories( ${CMAKE_SOURCE_DIR}/src/Utils )

# Setup output directories
set( LIBRARY_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}/bin
    CACHE
    PATH
    "Single directory for all libraries" )

set( EXECUTABLE_OUTPUT_PATH
    ${PROJECT_BINARY_DIR}/bin
    CACHE
    PATH
    "Single directory for all executables" )

set( MODELS_QRC "${CMAKE_SOURCE_DIR}/models.qrc" )
configure_file(
    "${PROJECT_SOURCE_DIR}/Configs.h.in"
    "${PROJECT_BINARY_DIR}/Configs.h" )

add_subdirectory( src )
add_subdirectory( demo )
