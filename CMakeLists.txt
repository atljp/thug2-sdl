cmake_minimum_required(VERSION 3.10)

project(thug2patch)

set(SRC "${CMAKE_SOURCE_DIR}/src")

set(SOURCE_FILES
	"${SRC}/main.cpp"
	"${SRC}/exports.def"
	"${SRC}/patch.cpp"
	"${SRC}/config.cpp"
	"${SRC}/displayconfig.cpp"
	"${SRC}/input.cpp"
	"${SRC}/Logger/Logger.cpp"
	"${SRC}/QB/LazyArray.cpp"
	"${SRC}/QB/LazyStruct.cpp"
	"${SRC}/QB/QB.cpp"
	"${SRC}/QB/script.cpp"
)

set(HEADER_FILES
	"${SRC}/d3d9.h"
	"${SRC}/patch.h"
	"${SRC}/config.h"
	"${SRC}/displayconfig.h"
	"${SRC}/input.h"
	"${SRC}/keycodes.h"
	"${SRC}/global.h"
	"${SRC}/Logger/Logger.h"
	"${SRC}/QB/cfuncs.h"
	"${SRC}/QB/LazyArray.h"
	"${SRC}/QB/LazyStruct.h"
	"${SRC}/QB/malloc.h"
	"${SRC}/QB/QB.h"
	"${SRC}/QB/script.h"
	"${SRC}/QB/scriptcontent.h"
)

find_package(SDL2 REQUIRED)
list(APPEND LIBS
	SDL2::SDL2
	SDL2::SDL2main
)
list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS})

add_library(partymod SHARED)

target_sources(partymod PRIVATE
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(partymod PUBLIC ${LIBS})

target_include_directories(partymod PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	${INCLUDES}
)

set_source_files_properties(exports.def PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(
    partymod
    PROPERTIES 
        OUTPUT_NAME "d3d9"
)