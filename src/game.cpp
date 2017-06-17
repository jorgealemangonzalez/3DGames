#include "game.h"
#include "utils.h"
#include "rendertotexture.h"
#include "scene.h"
#include "GUI.h"
#include "explosion.h"
#include "entity.h"

//some globals

RenderToTexture *rt = NULL;

Game *Game::instance = NULL;

Game::Game(SDL_Window *window) {
    this->window = window;
    instance = this;

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

    logger.open("log.txt", std::fstream::out);
    logger << "START\n";
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
    camera->lookAt(Vector3(0.f, 900.f, 25.f), Vector3(250.f, 900.f, 0.f),
                   Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
    camera->eye = Vector3(-550.f,3136.f,-2239);
    camera->setPerspective(70.f, window_width / (float) window_height, 0.1f,
                           100000000.f); //set the projection, we want to be perspectives

    human = new Human();
    enemy = new Enemy();

    std::cout<<"Load level1"<<std::endl;
    Scene* scene = Scene::getScene();
    scene->loadScene("../data/level1.txt");
    std::cout<<"Init finish";
    GUI* gui = GUI::getGUI();
}

//what to do when the image has to be draw
void Game::render(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set the clear color (the background color)
    //set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Put the camera matrices on the stack of OpenGL (only for fixed rendering)
    camera->set();
    //Draw out world

    Scene::getScene()->render(camera);
    Explosion::renderAll(camera);
    human->render(camera);

    //drawGrid(500); //background grid
    drawText(10.0,10.0,"Camera position: "+camera->eye.toString(),Vector3(234,26,34));

    GUI::getGUI()->render();
    glDisable(GL_BLEND);

    //example to render the FPS every 10 frames
    //drawText(2,2, std::to_string(fps), Vector3(1,1,1), 2 );

    //swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed) {
    Explosion::updateAll(seconds_elapsed);
    human->update(seconds_elapsed);
    enemy->update(seconds_elapsed);
    Scene::getScene()->update(seconds_elapsed);

    logger.close();
    logger.open("log.txt", std::fstream::out | std::ios::app);

    if(doLog)
        logger << "\nNext frame---------------------\n";
}

//Keyboard event handler (sync input)
void Game::onKeyPressed(SDL_KeyboardEvent event) {
    switch (event.keysym.sym) {
        case SDLK_ESCAPE:
            logger << "\nFINISH\n";
            logger.close();
            exit(0); //ESC key, kill the app

        case SDLK_1:
            human->cameraController->setMode(1);
            break;
        case SDLK_2:
            human->cameraController->setMode(2);
            break;
        case SDLK_3:
            human->cameraController->setMode(3);
            break;
        case SDLK_TAB:
            GUI::getGUI()->showHideGrid();
            break;
        case SDLK_c:
            Scene::getScene()->root->print(0);
            break;
        case SDLK_p: {
            UID uid;
            std::cin >> uid;
            std::vector<UID> uids;
            uids.push_back(uid);
            human->selectEntities(uids);
            break;
        }
        case SDLK_e:
            human->centerCameraOnControlling();
            break;

        case SDLK_o:
            debugMode = !debugMode;
            break;
        case SDLK_l:
            doLog = !doLog;
            break;
        case SDLK_i:
            for(auto &entry : Entity::s_entities){
                if(entry.second->stats.has_hp)
                    entry.second->stats.hp -= random()%1000;
            }
            break;
    }
}


void Game::onMouseButton(SDL_MouseButtonEvent event) {
    if (event.button == SDL_BUTTON_MIDDLE){ //middle mouse
        mouse_locked = !mouse_locked;
        SDL_ShowCursor(!mouse_locked);
    }else if (event.button == SDL_BUTTON_LEFT){
        mouse_when_press = Vector2(event.x, window_height-event.y);
        std::cout<<"press"<<mouse_when_press.x<<" "<<mouse_when_press.y<<std::endl;
    }
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event) {
    if (event.button == SDL_BUTTON_LEFT){
        if(GUI::getGUI()->show_grid){
            human->moveSelectedInPlane();
        }else {

            std::cout << "Test pointed\n";
            Vector2 mouse_when_up = Vector2(event.x, window_height - event.y);
            std::vector<UID> pointed = Entity::entityPointed(mouse_when_press, mouse_when_up, window_width,
                                                             window_height, camera);
            for (UID uid : pointed) {
                std::cout << "HAS APRETADO SOBRE LA ENTIDAD #" << uid << "\n";
            }
            if (mouse_when_press != mouse_when_up || pointed.size())
                human->selectEntities(pointed);
        }
    }else if(event.button == SDL_BUTTON_RIGHT){
        Vector3 pointingAt = camera->unproject(Vector3(event.x, window_height-event.y, 0), window_width, window_height);
        Vector3 direction = pointingAt - camera->eye;
        direction.normalize();
        human->moveSelectedInPlane();
    }
    mouse_when_press = Vector2(-1,-1);
}

void Game::onMouseWheel(SDL_MouseWheelEvent event) {
    //event.x: the amount scrolled horizontally, positive to the right and negative to the left
    //event.y: the amount scrolled vertically, positive away from the user and negative toward the user

    if (event.y > 0) Game::instance->camera->move(Vector3(0.0f, 0.0f, 1.0f) * event.y*100.f);
    if (event.y < 0) Game::instance->camera->move(Vector3(0.0f, 0.0f, -1.0f) * abs(event.y)*100.f);
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
    GUI::getGUI()->camera2d->setOrthographic(0, window_width, 0, window_height, 0, 1);
}

