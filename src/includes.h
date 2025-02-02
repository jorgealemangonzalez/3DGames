/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This file has all the includes so the app works in different systems.
	It is a little bit low level so do not worry about the code.
*/

#ifndef INCLUDES_H
#define INCLUDES_H

//under windows we need this file to make opengl work
#ifdef WIN32 
	#include <windows.h>
#endif

#ifndef APIENTRY
    #define APIENTRY
#endif

#ifdef WIN32
	#define USE_GLEW
	#define GLEW_STATIC
	#include <GL/glew.h>
	#pragma comment(lib, "glew32s.lib")
#endif


//SDL
//#pragma comment(lib, "SDL2.lib")
//#pragma comment(lib, "SDL2main.lib")


#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

//GLUT
#ifdef WIN32
    #include "GL/GLU.h"
	//#include <GL/glut.h>
#endif

#ifdef __APPLE__
    #include "OpenGL/glu.h"
	//#include <GLUT/glut.h>
#endif

#ifndef WIN32
#ifndef __APPLE__
    #include <GL/glext.h>
#endif
#endif

#include <iostream>

//remove warnings

//used to access opengl extensions
#define REGISTER_GLEXT(RET, FUNCNAME, ...) typedef RET ( * FUNCNAME ## _func)(__VA_ARGS__); FUNCNAME ## _func FUNCNAME = NULL; 
#define IMPORT_GLEXT(FUNCNAME) FUNCNAME = (FUNCNAME ## _func) SDL_GL_GetProcAddress(#FUNCNAME); if (FUNCNAME == NULL) { std::cout << "ERROR: This Graphics card doesnt support " << #FUNCNAME << std::endl; }

#define ABS(x) ( x < 0 ? -x : x)
#define MAX(a,b) ( a < b ? b : a)
#define MIN(a,b) ( a > b ? b : a)
#define PI 3.14159
#define HUMAN_TEAM "t1"
#define ENEMY_TEAM "t2"
#define NEUTRAL_TEAM "neutral"
#define NO_TEAM "no_team"
#define ANY_TEAM ""

extern bool debugMode;  //Global declarada en main

//OPENGL EXTENSIONS




#endif
