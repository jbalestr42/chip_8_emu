cmake_minimum_required(VERSION 3.8)

project(chip_8_emu)

set(HEADER_FILES
	include/${PROJECT_NAME}/Chip8.hpp
	include/${PROJECT_NAME}/CPU.hpp
	include/${PROJECT_NAME}/Memory.hpp
	include/${PROJECT_NAME}/Display.hpp
)

set(SOURCE_FILES
	source/main.cpp
	source/Chip8.cpp
	source/CPU.cpp
	source/Memory.cpp
	source/Display.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${SOURCE_FILES})

add_executable(${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	sfml-audio
	sfml-graphics
	sfml-system
	sfml-window
)

target_include_directories(${PROJECT_NAME} PRIVATE
	include/${PROJECT_NAME}
)

if (WIN32 AND SFML_BUILD_AUDIO)
	add_custom_command(TARGET chip_8_emu POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}\\..\\external\\SFML\\extlibs\\bin\\x64\\openal32.dll" $<TARGET_FILE_DIR:chip_8_emu>)
endif()
