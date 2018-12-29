cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VITASDK})
		set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
	else()
		message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
	endif()
endif()

project("badapplevita")
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Bad Apple Vita!!!")
set(VITA_TITLEID "BADA00001")
set(VITA_VERSION "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g3")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(badapplevita
	src/main.c
	src/monorale.c
	src/sound.c
)

target_link_libraries(badapplevita
	SceDisplay_stub
	SceAudio_stub
	vorbisenc
	vorbisfile
	vorbis
	ogg
	m
)

vita_create_self(
	badapple_VITA.self badapplevita
)

vita_create_vpk(
	badapple_VITA.vpk ${VITA_TITLEID} badapple_VITA.self
	VERSION ${VITA_VERSION}
	NAME ${VITA_APP_NAME}
	FILE bin/monorale.bin monorale.bin	
	FILE bin/sce_sys/icon0.png sce_sys/icon0.png
	FILE bin/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
	FILE bin/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
	FILE bin/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
