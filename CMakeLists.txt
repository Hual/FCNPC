cmake_minimum_required (VERSION 3.5)

project (FCNPC)

# Definintions
set (INCLUDE_VERSION 207)
set (PLUGIN_VERSION "2.0.8")
add_definitions (-DPLUGIN_VERSION="${PLUGIN_VERSION}" -DINCLUDE_VERSION=${INCLUDE_VERSION})

set (CMAKE_SHARED_LIBRARY_PREFIX "")
set (CMAKE_DEBUG_POSTFIX "")
set (CMAKE_RELWITHDEBINFO_POSTFIX "")
set (CMAKE_MINSIZEREL_POSTFIX "")

set (LIBRARY_GROUP "lib\\")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions (-DWIN32 -D_WIN32 -D__STDC__ -DSYSTEM_NAME="Windows")

	# *.def
	file (GLOB EXPORTS_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/Exports.def
	)
	file (GLOB EXPORTS_DL_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/ExportsDL.def
	)

	# snprintf
	file (GLOB SNPRINTF_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/lib/snprintf/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/lib/snprintf/*.h
	)
	set (PLATFORM_SRC ${SNPRINTF_SRC})
	source_group("${LIBRARY_GROUP}snprintf" FILES ${SNPRINTF_SRC})

	# flags
	set (USE_MSVC_RUNTIME_LIBRARY_DLL true)
	set (CMAKE_C_FLAGS "/W4")
	set (CMAKE_CXX_FLAGS "/W4 /EHsc")

	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
	set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MTd")
	set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")

	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MTd")
	set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")

	set (CMAKE_SHARED_LINKER_FLAGS "/NOLOGO /MACHINE:X86")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_definitions (-DLINUX -DSYSTEM_NAME="GNU/Linux")

	# fopen_s
	file (GLOB FOPEN_S_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/lib/fopen_s/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/lib/fopen_s/*.h
	)
	set (PLATFORM_SRC ${FOPEN_S_SRC})
	source_group("${LIBRARY_GROUP}fopen_s" FILES ${FOPEN_S_SRC})

	# flags
	set (FLAGS "-m32 -fPIC -O2 -fno-stack-protector -Wall -Wno-attributes -Wno-strict-aliasing -Wno-sequence-point")
	set (CMAKE_C_FLAGS ${FLAGS})
	set (CMAKE_CXX_FLAGS "${FLAGS} -std=c++11")
	set (CMAKE_SHARED_LINKER_FLAGS "-fshort-wchar -shared -static-libgcc -static-libstdc++")
endif ()

# open.mp
add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/lib/open.mp-sdk
)
add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/lib/open.mp-network
)

# Bullet
set (BUILD_CPU_DEMOS OFF CACHE BOOL "Build original Bullet CPU examples")
set (BUILD_BULLET2_DEMOS OFF CACHE BOOL "Set when you want to build the Bullet 2 demos")
set (BUILD_EXTRAS OFF CACHE BOOL "Set when you want to build the extras")
set (BUILD_OPENGL3_DEMOS OFF CACHE BOOL "Set when you want to build the OpenGL3+ demos")
set (BUILD_UNIT_TESTS OFF CACHE BOOL "Build Unit Tests")
add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3
)

set (BULLET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3")
find_path(BULLET_INCLUDE_DIRS NAMES btBulletCollisionCommon.h
  HINTS
    ${BULLET_ROOT}/include
    ${BULLET_ROOT}/src
  PATH_SUFFIXES bullet
)

# include (FindBullet)

# FCNPC
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	${CMAKE_CURRENT_SOURCE_DIR}/lib/sdk
	${BULLET_INCLUDE_DIRS}
)

file (GLOB FCNPC_SRC
	${FCNPC_SRC}
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
source_group("" FILES ${FCNPC_SRC})

# SDK
file (GLOB SDK_SRC
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdk/*.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdk/*.h
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdk/amx/*.c
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/sdk/amx/*.h
)
source_group("${LIBRARY_GROUP}SDK" FILES ${SDK_SRC})

# strlcpy
file (GLOB STRLCPY_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/lib/strlcpy/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/lib/strlcpy/*.h
)
source_group("${LIBRARY_GROUP}strlcpy" FILES ${STRLCPY_SRC})

# Include file
configure_file(src/FCNPC.inc.in ${CMAKE_CURRENT_SOURCE_DIR}/sampsvr_files/FCNPC.inc NEWLINE_STYLE CRLF)

# SAMP03DL
add_library (${PROJECT_NAME}-DL SHARED ${FCNPC_SRC}
                                       ${EXPORTS_DL_SRC}
                                       ${STRLCPY_SRC}
                                       ${PLATFORM_SRC})
target_compile_definitions(${PROJECT_NAME}-DL PRIVATE -DSAMP_03DL -DOMP_WRAPPER)

target_link_libraries (${PROJECT_NAME}-DL
	OMP-SDK
	OMP-NetCode

	BulletInverseDynamics
	BulletSoftBody
	BulletDynamics
	BulletCollision
	LinearMath
	Bullet3Common
	)
