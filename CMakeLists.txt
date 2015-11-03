project( SignProcessing )
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

# OpenGL
find_package( OpenGL REQUIRED )

# OpenMP
find_package( OpenMP )
if( OPENMP_FOUND )
	option( WITH_OPENMP "Whether to use parallel processing capabilities of OPENMP. ON/OFF" ON )
endif( OPENMP_FOUND )

if( OPENMP_FOUND AND WITH_OPENMP )
	message( STATUS "With OpenMP " )
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAX_OPENMP_NUM_THREADS=${PROCESSOR_COUNT} -DOPENMP_NUM_THREADS=${PROCESSOR_COUNT} ${OpenMP_CXX_FLAGS} -DOPENMP" )
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMAX_OPENMP_NUM_THREADS=${PROCESSOR_COUNT} -DOPENMP_NUM_THREADS=${PROCESSOR_COUNT} ${OpenMP_CXX_FLAGS} ${OpenMP_C_FLAGS} -DOPENMP" )
else( OPENMP_FOUND AND WITH_OPENMP )
	message( STATUS "Without OpenMP" )
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAX_OPENMP_NUM_THREADS=1 -DOPENMP_NUM_THREADS=1" )
	set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMAX_OPENMP_NUM_THREADS=1 -DOPENMP_NUM_THREADS=1" )
endif( OPENMP_FOUND AND WITH_OPENMP )

# Boost
set( Boost_USE_STATIC_LIBS OFF )
set( Boost_USE_MULTITHREAD ON )
find_package( Boost REQUIRED COMPONENTS filesystem thread )
message( STATUS "Boost_LIBS: ${Boost_LIBRARIES}" )

# OpenCV 
find_package( OpenCV REQUIRED )

# Qt4 
find_package( Qt4 REQUIRED )
set( QT_USE_QTMAIN TRUE )
set( QT_USE_QTOPENGL TRUE )
set( QT_USE_QTXML TRUE )
set( QT_USE_QTNETWORK TRUE )
set( QT_USE_PHONON TURE )

# include them all
include( ${QT_USE_FILE} )
include_directories( ${OPENCV_INCLUDE_DIR} )

include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/src )
include_directories( ${CMAKE_BINARY_DIR}/src/Utils )
include_directories( ${CMAKE_SOURCE_DIR}/src/Utils )
include_directories( ${CMAKE_BINARY_DIR}/src/SignCutter )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignCutter )
include_directories( ${CMAKE_BINARY_DIR}/src/SignTransformer )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignTransformer )
include_directories( ${CMAKE_BINARY_DIR}/src/SignLogger )
include_directories( ${CMAKE_SOURCE_DIR}/src/SignLogger )

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

add_subdirectory( src )
add_subdirectory( demo )