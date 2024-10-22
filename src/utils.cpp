#include "utils.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

#include "includes.h"
#include "game.h"
#include "extra/stb_easy_font.h"

long getTime()
{
	#ifdef WIN32
		return GetTickCount();
	#else
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (int)(tv.tv_sec*1000 + (tv.tv_usec / 1000));
	#endif
}

//Draw the grid
void drawGrid(float dist)
{
	int num_lines = 20;
	glLineWidth(1);
	glColor3f(0.5,0.5,0.5);
	glDisable( GL_TEXTURE_2D );
	glBegin( GL_LINES );
		for (int i = 0; i <= int(num_lines * 0.5); ++i)
		{
			float a = dist * num_lines * 0.5f;
			float b = i * dist;

			if (i == num_lines * 0.5)
				glColor3f(1,0.25,0.25);
			else if (i%2)
				glColor3f(0.25,0.25,0.25);
			else
				glColor3f(0.5,0.5,0.5);


			glVertex3f(a,b,-a);
			glVertex3f(-a,b,-a);
			glVertex3f(a,-b,-a);
			glVertex3f(-a,-b,-a);
			glVertex3f(b,a,-a);
			glVertex3f(b,-a,-a);
			glVertex3f(-b,a,-a);
			glVertex3f(-b,-a,-a);
			glVertex3f(a,b,a);
			glVertex3f(-a,b,a);
			glVertex3f(a,-b,a);
			glVertex3f(-a,-b,a);
			glVertex3f(b,a,a);
			glVertex3f(b,-a,a);
			glVertex3f(-b,a,a);
			glVertex3f(-b,-a,a);


			glVertex3f(a,-a,b);
			glVertex3f(-a,-a,b);
			glVertex3f(a,-a,-b);
			glVertex3f(-a,-a,-b);

			glVertex3f(b,-a,a);
			glVertex3f(b,-a,-a);
			glVertex3f(-b,-a,a);
			glVertex3f(-b,-a,-a);

			glVertex3f(-a, a,b);
			glVertex3f(-a, -a,b);
			glVertex3f(-a, a,-b);
			glVertex3f(-a, -a,-b);
			glVertex3f(-a, b,a);
			glVertex3f(-a, b,-a);
			glVertex3f(-a, -b,a);
			glVertex3f(-a, -b,-a);
			glVertex3f(a, a,b);
			glVertex3f(a, -a,b);
			glVertex3f(a, a,-b);
			glVertex3f(a, -a,-b);
			glVertex3f(a, b,a);
			glVertex3f(a, b,-a);
			glVertex3f(a, -b,a);
			glVertex3f(a, -b,-a);
		}
	glEnd();
	glColor3f(1,1,1);
}

//this function is used to access OpenGL Extensions (special features not supported by all cards)
void* getGLProcAddress(const char* name)
{
	return SDL_GL_GetProcAddress(name);
}

//Retrieve the current path of the application
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef WIN32
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

std::string getPath()
{
    std::string fullpath;
    // ----------------------------------------------------------------------------
    // This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__e444
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
    chdir(path);
    fullpath = path;
#else
	 char cCurrentPath[1024];
	 if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		 return "";

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	fullpath = cCurrentPath;

#endif    
    return fullpath;
}


bool checkGLErrors()
{
	#ifdef _DEBUG
	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		std::cerr << "OpenGL Error: " << errString << std::endl;
		return false;
	}
	#endif

	return true;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


Vector2 getDesktopSize( int display_index )
{
  SDL_DisplayMode current;
  // Get current display mode of all displays.
  int should_be_zero = SDL_GetCurrentDisplayMode(display_index, &current);
  return Vector2( (float)current.w, (float)current.h );
}


bool drawText(float x, float y, std::string text, Vector3 c, float scale )
{
	static char buffer[99999]; // ~500 chars
	int num_quads;

	if (scale == 0)
		return true;

	x /= scale;
	y /= scale;

	num_quads = stb_easy_font_print(x, y, (char*)(text.c_str()), NULL, buffer, sizeof(buffer));

	Camera cam;
	cam.setOrthographic(0, Game::instance->window_width / scale, Game::instance->window_height / scale, 0, -1, 1);
	cam.set();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glColor3f(c.x, c.y, c.z);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buffer);
	glDrawArrays(GL_QUADS, 0, num_quads * 4);
	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
}
