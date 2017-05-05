#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "rendertotexture.h"
#include "shader.h"
#include "entity.h"
#include "level.h"

#include <cmath>

//some globals
Entity *root = NULL;

RenderToTexture *rt = NULL;

Game *Game::instance = NULL;

Game::Game(SDL_Window *window) {
    this->window = window;
    instance = this;

    controller = new CameraController();

    // initialize attributes
    // Warning: DO NOT CREATE STUFF HERE, USE THE INIT
    // things create here cannot access opengl
    SDL_GetWindowSize(window, &window_width, &window_height);
    std::cout << " * Window size: " << window_width << " x " << window_height << std::endl;

    fps = 0;
    frame = 0;
    time = 0.0f;
    elapsed_time = 0.0f;
    angle = 0;

    keystate = NULL;
    mouse_locked = false;
}

//Here we have already GL working, so we can create meshes and textures
void Game::init(void) {
    std::cout << " * Path: " << getPath() << std::endl;

    //SDL_SetWindowSize(window, 50,50);

    //OpenGL flags
    glEnable(GL_CULL_FACE); //render both sides of every triangle
    glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

    //create our camera
    camera = new Camera();
    camera->lookAt(Vector3(0.f, 25.f, 25.f), Vector3(0.f, 0.f, 0.f),
                   Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
    camera->setPerspective(70.f, window_width / (float) window_height, 0.1f,
                           100000000.f); //set the projection, we want to be perspectives

    root = new Entity();

    std::cout<<"Load level1"<<std::endl;
    level = new Level();
    level->load("../data/level1.txt",root);
    std::cout<<"Init finish";
}

//what to do when the image has to be draw
void Game::render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Put the camera matrices on the stack of OpenGL (only for fixed rendering)
    camera->set();
    //Draw out world
    drawGrid(500); //background grid

    root->render(camera);

    glDisable(GL_BLEND);



    //example to render the FPS every 10 frames
    //drawText(2,2, std::to_string(fps), Vector3(1,1,1), 2 );

    //swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed) {
    controller->update(seconds_elapsed);
    level->update(seconds_elapsed);

}

//Keyboard event handler (sync input)
void Game::onKeyPressed(SDL_KeyboardEvent event) {
    switch (event.keysym.sym) {
        case SDLK_ESCAPE:
            exit(0); //ESC key, kill the app

        case SDLK_1:
            controller->setMode(1);
            break;
        case SDLK_2:
            controller->setMode(2);
            break;
    }
}


void Game::onMouseButton(SDL_MouseButtonEvent event) {
    if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
    {
        mouse_locked = !mouse_locked;
        SDL_ShowCursor(!mouse_locked);
    }
}

void Game::setWindowSize(int width, int height) {
    std::cout << "window resized: " << width << "," << height << std::endl;

    /*
    Uint32 flags = SDL_GetWindowFlags(window);
    if(flags & SDL_WINDOW_ALLOW_HIGHDPI)
    {
        width *= 2;
        height *= 2;
    }
    */

    glViewport(0, 0, width, height);
    camera->aspect = width / (float) height;
    window_width = width;
    window_height = height;
}

