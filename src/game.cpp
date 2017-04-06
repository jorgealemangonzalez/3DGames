#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "rendertotexture.h"
#include "shader.h"

#include <cmath>

//some globals
Mesh* mesh = NULL;
Mesh* mesh_ase = NULL;	//TODO Quit later
Texture* texture = NULL;
Shader* shader = NULL;
float angle = 0;
RenderToTexture* rt = NULL;

Game* Game::instance = NULL;

Game::Game(SDL_Window* window)
{
	this->window = window;
	instance = this;

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	SDL_GetWindowSize( window, &window_width, &window_height );
	std::cout << " * Window size: " << window_width << " x " << window_height << std::endl;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;

	keystate = NULL;
	mouse_locked = false;
}

//Here we have already GL working, so we can create meshes and textures
void Game::init(void)
{
    std::cout << " * Path: " << getPath() << std::endl;
    
    //SDL_SetWindowSize(window, 50,50);

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,25.f,25.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective
	//create a plane mesh
	mesh = new Mesh();
	mesh->createPlane(10);

	mesh_ase = new Mesh();
	mesh_ase->loadASE("../data/meshes/p38.ASE");
	/*
	shader = new Shader();
	if( !shader->load("data/shaders/simple.vs","data/shaders/simple.fs") )
	{
		std::cout << "shader not found or error" << std::endl;
		exit(0);
	}
	*/

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Game::render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	camera->set();

	//Draw out world
	drawGrid(500); //background grid
    
	//create model matrix from plane
	Matrix44 m;
    m.setScale(1.0f, 1.0f, 1.0f);
	m.rotate( (float)(angle * DEG2RAD), Vector3(0.0f,1.0f, 0.0f) ); //build a rotation matrix

	//draw the plane
	if(shader) //render using shader (enable to test shader)
	{
	    Matrix44 mvp = m * camera->viewprojection_matrix;
		shader->enable();
		shader->setMatrix44("u_model", m );
		shader->setMatrix44("u_mvp", mvp );
   
		mesh->render(GL_TRIANGLES, shader);
		shader->disable();
	}
	else //render using fixed pipeline (DEPRECATED, use for debug only)
	{
		glPointSize(5);
		glPushMatrix();
		m.multGL();
        std::vector<Vector3> &v = mesh_ase->vertices;
        //std::cout<<"------------"<<std::endl;
		for(int i = 0 ;i < v.size(); ++i ){
            //std::cout<<v[i].x<< " " << v[i].y <<" "<< v[i].z<<std::endl;
		}
		mesh_ase->render(GL_TRIANGLES);
		glPopMatrix();
	}
    
    glDisable( GL_BLEND );

	//example to render the FPS every 10 frames
//	drawText(2,2, std::to_string(fps), Vector3(1,1,1), 2 );

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

	//mouse input to rotate the cam
	if ((mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		camera->rotate(mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	//async input to move the camera around
	if(keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
	if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP]) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_DOWN]) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT]) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT]) camera->move(Vector3(-1.0f,0.0f, 0.0f) * speed);
    
	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
	{
		int center_x = (int)floor(window_width*0.5f);
		int center_y = (int)floor(window_height*0.5f);
        //center_x = center_y = 50;
		SDL_WarpMouseInWindow(this->window, center_x, center_y); //put the mouse back in the middle of the screen
		//SDL_WarpMouseGlobal(center_x, center_y); //put the mouse back in the middle of the screen
        
        this->mouse_position.x = (float)center_x;
        this->mouse_position.y = (float)center_y;
	}
    

	angle += (float)seconds_elapsed * 10.0f;
}

//Keyboard event handler (sync input)
void Game::onKeyPressed( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: exit(0); //ESC key, kill the app
	}
}


void Game::onMouseButton( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::setWindowSize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
    
	/*
    Uint32 flags = SDL_GetWindowFlags(window);
    if(flags & SDL_WINDOW_ALLOW_HIGHDPI)
    {
        width *= 2;
        height *= 2;
    }
	*/

	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

