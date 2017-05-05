cmake_minimum_required(VERSION 3.7)
project(TJE_framework_2017)

set(CMAKE_CXX_STANDARD 11)
set(CFLAGS "-g -Wall")
set(CXXFLAGS "-g -Wall")
set(SDL_LIB "-lSDL2")
set(GLUT_LIB "-lGL -lGLU")
set(LIBS "${SDL_LIB} ${GLUT_LIB}")

if(NOT APPLE)
    set(CMAKE_CXX_FLAGS_RELEASE  "-g -Wall -lSDL2 -lGL -lGLU")
    set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -lSDL2 -lGL -lGLU")
endif()
if(APPLE)
    set(CMAKE_CXX_FLAGS_RELEASE  "-g -Wall -lSDL2 -framework OpenGL")
    set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -lSDL2 -framework OpenGL")
endif()

set(SOURCE_FILES
        src/extra/stb_easy_font.h
        src/extra/textparser.cpp
        src/extra/textparser.h
        src/camera.cpp
        src/camera.h
        src/framework.cpp
        src/framework.h
        src/game.cpp
        src/game.h
        src/includes.h
        src/main.cpp
        src/mesh.cpp
        src/mesh.h
        src/rendertotexture.cpp
        src/rendertotexture.h
        src/shader.cpp
        src/shader.h
        src/texture.cpp
        src/texture.h
        src/utils.cpp
        src/utils.h src/entity.cpp src/entity.h src/game_entities.h src/game_entities.cpp src/level.cpp src/level.h src/controller.h src/controller.cpp
        src/extra/coldet/src/box.cpp
        src/extra/coldet/src/box.h
        src/extra/coldet/src/box_bld.cpp
        src/extra/coldet/src/cdmath3d.cpp
        src/extra/coldet/src/cdmath3d.h
        src/extra/coldet/src/coldet.cpp
        src/extra/coldet/src/coldet.h
        src/extra/coldet/src/coldet_bld.cpp
        src/extra/coldet/src/coldetimpl.h
        src/extra/coldet/src/multi_impl.h
        src/extra/coldet/src/multiobject.cpp
        src/extra/coldet/src/multiobject.h
        src/extra/coldet/src/mytritri.cpp
        src/extra/coldet/src/mytritri.h
        src/extra/coldet/src/sweep_prune.h
        src/extra/coldet/src/sysdep.h
        src/extra/coldet/src/sysdep.cpp
        src/extra/coldet/src/tritri.c
        src/player.cpp src/player.h)

add_executable(TJE_framework_2017 ${SOURCE_FILES})
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(SDL2_LIBRARIES "-L${SDL2_LIBDIR} -lSDL2")
target_link_libraries(TJE_framework_2017 ${SDL2_LIBRARIES})
target_link_libraries(TJE_framework_2017 ${OPENGL_LIBRARIES})