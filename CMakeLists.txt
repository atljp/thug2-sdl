cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME_VAR "THUG2_SDL")
project(${PROJECT_NAME_VAR})

set(SRC "${CMAKE_SOURCE_DIR}/src")

set(SOURCE_FILES
	"${SRC}/main.cpp"
	"${SRC}/exports.def"
	"${SRC}/Resources/resources.rc"
	"${SRC}/patch.cpp"
	"${SRC}/config.cpp"
	"${SRC}/displayconfig.cpp"
	"${SRC}/input.cpp"
	"${SRC}/modloader.cpp"
	"${SRC}/Mod/modcontent.cpp"
	"${SRC}/Logger/Logger.cpp"
	"${SRC}/Logger/ErrorManager.cpp"
	"${SRC}/QB/CFuncs.cpp"
	"${SRC}/QB/LazyArray.cpp"
	"${SRC}/QB/LazyStruct.cpp"
	"${SRC}/QB/QB.cpp"
	"${SRC}/QB/script.cpp"
	"${SRC}/event.cpp"
	"${SRC}/Util/sb.cpp"
)

set(HEADER_FILES
	"${SRC}/d3d9.h"
	"${SRC}/patch.h"
	"${SRC}/config.h"
	"${SRC}/displayconfig.h"
	"${SRC}/input.h"
	"${SRC}/modloader.h"
	"${SRC}/Mod/modcontent.h"
	"${SRC}/keycodes.h"
	"${SRC}/global.h"
	"${SRC}/Logger/Logger.h"
	"${SRC}/Logger/ErrorManager.h"
	"${SRC}/QB/CFuncs.h"
	"${SRC}/QB/LazyArray.h"
	"${SRC}/QB/LazyStruct.h"
	"${SRC}/QB/malloc.h"
	"${SRC}/QB/QB.h"
	"${SRC}/QB/script.h"
	"${SRC}/QB/scriptcontent.h"
	"${SRC}/Resources/resources.h"	
	"${SRC}/event.h"
	"${SRC}/Util/sb.h"
)

find_package(SDL2 REQUIRED)
list(APPEND LIBS
	SDL2::SDL2
	SDL2::SDL2main
)
list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS})

add_library(${PROJECT_NAME_VAR} SHARED)

target_sources(${PROJECT_NAME_VAR} PRIVATE
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME_VAR} PUBLIC ${LIBS} dbghelp)

target_include_directories(${PROJECT_NAME_VAR} PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	${INCLUDES}
)

set_source_files_properties(exports.def PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(
    ${PROJECT_NAME_VAR}
    PROPERTIES 
        OUTPUT_NAME "thug2sdl"
)

add_executable(t2sdl-launcher)

set(LSRC "${CMAKE_SOURCE_DIR}/launcher")

set(LSOURCE_FILES
	"${LSRC}/main.c"
	"${LSRC}/pgui.c"
	"${LSRC}/pagegeneral.c"
	"${LSRC}/pagekeyboard.c"
	"${LSRC}/pagegamepad.c"
	"${LSRC}/pagecontrol.c"
	"${LSRC}/Resources.rc"
)

set (LHEADER_FILES
	"${LSRC}/pgui.h"
	"${LSRC}/pagegeneral.h"
	"${LSRC}/pagekeyboard.h"
	"${LSRC}/pagegamepad.h"
	"${LSRC}/pagecontrol.h"
	"${LSRC}/global.h"
	"${LSRC}/Resources.h"
)

target_sources(t2sdl-launcher PUBLIC 
	${LSOURCE_FILES}
	${LHEADER_FILES}
)
target_link_libraries(t2sdl-launcher PRIVATE 
	${LIBS}
	comctl32.lib
	UxTheme.lib
)
target_include_directories(t2sdl-launcher PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	${INCLUDES}
)

add_executable(thug2sdlpatcher)

target_sources(thug2sdlpatcher PUBLIC
	"${CMAKE_SOURCE_DIR}/patcher/main.c"
)

target_include_directories(thug2sdlpatcher PUBLIC
	"${CMAKE_SOURCE_DIR}/src"
	${THIRDPARTY}
)