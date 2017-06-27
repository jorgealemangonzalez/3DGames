#include "game.h"
#include "utils.h"
#include "rendertotexture.h"
#include "scene.h"
#include "GUI.h"
#include "explosion.h"
#include "entity.h"
#include "MusicManager.h"

//some globals

RenderToTexture *rt = NULL;

Game *Game::instance = NULL;

Game::~Game(){

    /*
    //Mesh::deleteStaticMeshesPointers();
    //Texture::deleteStaticTexturePointers();
    delete camera;
    delete human;
    delete enemy;
    //delete Scene::getScene();
    delete GUI::getGUI();
    delete BulletManager::getManager();
    MusicManager::stop_pool();
     */


}

void Game::resetGame(){
    MusicManager::stop_pool();
    BulletManager::getManager()->last_pos_pool = 0;
    delete human;
    delete enemy;
    delete camera;
    Entity::destroy_all();
    delete Scene::getScene();
    delete GUI::getGUI();

    this->init();
}

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
    pause = false;
    mouse_locked = false;
    mouseLeft = false;
    mouseRight = false;

    selectedLevel = LEVEL_TUTORIAL;
}

//Here we have already GL working, so we can create meshes and textures
void Game::init(void) {
    std::cout << " * Path: " << getPath() << std::endl;

    this->gameState = MAIN_MENU;

    //music
    MusicManager::init();

    MusicManager::ambientTense.play();
    //SDL_SetWindowSize(window, 50,50);

    //OpenGL flags
    glEnable(GL_CULL_FACE); //render both sides of every triangle
    glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

    //create our camera
    camera = new Camera();
    camera->lookAt(Vector3(0.f, 900.f, 25.f), Vector3(250.f, 900.f, 0.f),
                   Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
    camera->eye = Vector3(12.f,1050.f,135.f);
    camera->setPerspective(70.f, window_width / (float) window_height, 0.1f,
                           100000000.f); //set the projection, we want to be perspectives

    human = new Human();
    enemy = new Enemy();

    std::cout<<"Load level: "<<selectedLevel<<std::endl;
    Scene* scene = Scene::getScene();
    scene->loadScene(selectedLevel.c_str());
    std::cout<<"Init finish\n";
    GUI* gui = GUI::getGUI();
}

//what to do when the image has to be draw
void Game::render(void) {
    //set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera->set();

    Scene::getScene()->render(camera);
    Explosion::renderAll(camera);
    human->render(camera);

    GUI::getGUI()->render();
    float y = window_height - mouse_position.y;

    if(this->gameState == PLAYING) {

    }
    else if(this->gameState == GAME_OVER) {
        drawText(100, 100, humanWins ? "GAME OVER! HAS GANADO!" : "GAME OVER! HAS PERDIDO...", Vector3(1,1,0), 4);
        drawText(100, 140, (y > 140 && y < 180) ? ">MENU PRINCIPAL" : "MENU PRINCIPAL", Vector3(1,1,0), 4);
        drawText(100, 180, (y > 180 && y < 220) ? ">SALIR" : "SALIR", Vector3(1,1,0), 4);

    }
    else if(this->gameState == MENU) {
        drawText(100, 100, (y > 100 && y < 140) ? ">REANUDAR PARTIDA" : "REANUDAR PARTIDA", Vector3(1,1,0), 4);
        drawText(100, 140, (y > 140 && y < 180) ? ">MENU PRINCIPAL" : "MENU PRINCIPAL", Vector3(1,1,0), 4);
        drawText(100, 180, (y > 180 && y < 220) ? ">SALIR" : "SALIR", Vector3(1,1,0), 4);

    }else if(this->gameState == MAIN_MENU) {
        drawText(140, 100, "SPACE THINGS!", Vector3(1,1,0), 8);
        drawText(100, 180, (y > 180 && y < 220) ? ">JUGAR" : "JUGAR", Vector3(1,1,0), 4);
        drawText(100, 220, (y > 220 && y < 260) ? ">NIVELES" : "NIVELES", Vector3(1,1,0), 4);
        drawText(100, 260, (y > 260 && y < 300) ? ">SALIR" : "SALIR", Vector3(1,1,0), 4);
    }else if(this->gameState == LEVEL) {
        drawText(100, 100, (selectedLevel == LEVEL_TUTORIAL ? "*TUTORIAL" : (y > 100 && y < 140) ? ">TUTORIAL" : "TUTORIAL"), Vector3(1,1,0), 4);
        drawText(100, 140, (selectedLevel == LEVEL_NIVEL1 ? "*NIVEL 1" : (y > 140 && y < 180) ? ">NIVEL 1" : "NIVEL 1"), Vector3(1,1,0), 4);
        drawText(100, 180, (selectedLevel == LEVEL_DEBUG ? "*NIVEL DEBUG" : (y > 180 && y < 220) ? ">NIVEL DEBUG" : "NIVEL DEBUG"), Vector3(1,1,0), 4);
        drawText(100, 220, (y > 220 && y < 260) ? ">VOLVER" : "VOLVER", Vector3(1,1,0), 4);
    }
    glDisable(GL_BLEND);
    //swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed) {
    if(this->gameState == PLAYING) {
        if (!pause) {
            Explosion::updateAll(seconds_elapsed);
            enemy->update(seconds_elapsed);
            Scene::getScene()->update(seconds_elapsed);
        }
        Scene::getScene()->updateGUI();
        MusicManager::update();
        human->update(seconds_elapsed);

        if (!keystate[SDL_SCANCODE_LSHIFT]) {
            GUI *gui = GUI::getGUI();
            if (mouseLeft) {
                gui->addPlane(mouse_when_press, mouse_position);
                gui->addLine(mouse_when_press, Vector2(mouse_when_press.x, mouse_position.y), Vector4(1, 1, 1, 1),
                             true);
                gui->addLine(mouse_position, Vector2(mouse_when_press.x, mouse_position.y), Vector4(1, 1, 1, 1), true);
                gui->addLine(mouse_when_press, Vector2(mouse_position.x, mouse_when_press.y), Vector4(1, 1, 1, 1),
                             true);
                gui->addLine(mouse_position, Vector2(mouse_position.x, mouse_when_press.y), Vector4(1, 1, 1, 1), true);
            } else if (mouseRight) {
                Vector3 selectedMove;
                if (human->getPositionSelectedMove(selectedMove)) {
                    Vector3 selectedMoveP = camera->project(selectedMove, window_width, window_height);
                    gui->addLine(mouse_when_press, selectedMoveP, Vector4(1, 1, 1, 0.5), true);
                    gui->addCenteredCircles(selectedMove, human->getRadiusControlling());
                }
            }
        }
    }
    else if(this->gameState == MENU){

    }
}

//Keyboard event handler (sync input)
void Game::onKeyPressed(SDL_KeyboardEvent event) {
    if(this->gameState == PLAYING) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                gameState = MENU;
                break;
            case SDLK_c:
                Scene::getScene()->root->print(0);
                break;
            case SDLK_a:
                human->selectAllEntities();
                break;
            case SDLK_e:
                human->centerCameraOnControlling();
                break;
            case SDLK_SPACE:
                pause = !pause;
                break;
            case SDLK_o:
                debugMode = !debugMode;
                break;
            case SDLK_i:
                for (auto &entry : Entity::s_entities) {
                    if (entry.second->stats.has_hp)
                        entry.second->stats.hp -= random() % 1000;
                }
                break;
        }
    }
    else if(this->gameState == GAME_OVER) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                std::cout<< "\nFINISH\n";
                exit(0);
            case SDLK_r:
                this->gameState = RESET;
        }
    }
    else if(this->gameState == MENU) {
        switch (event.keysym.sym) {
            case SDLK_ESCAPE:
                gameState = PLAYING;
                break;
        }
    }
}


void Game::onMouseButton(SDL_MouseButtonEvent event) {
    if(this->gameState == PLAYING) {
        mouse_when_press = Vector2(event.x, window_height - event.y);
        if (event.button == SDL_BUTTON_MIDDLE) { //middle mouse
            mouse_locked = !mouse_locked;
            SDL_ShowCursor(!mouse_locked);
        } else if (event.button == SDL_BUTTON_LEFT) {
            mouseLeft = true;
        } else if (event.button == SDL_BUTTON_RIGHT) {
            mouseRight = true;
            human->updateCenter = false;
        }
    }
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event) {
    if(this->gameState == PLAYING) {
        mouseLeft = false;
        mouseRight = false;
        Vector2 mouse_when_up = Vector2(event.x, window_height - event.y);

        if (!keystate[SDL_SCANCODE_LSHIFT]) {
            if (event.button == SDL_BUTTON_LEFT) {
                std::vector<UID> pointed = Entity::entityPointed(mouse_when_press, mouse_when_up, window_width,
                                                                 window_height, camera);
                if (mouse_when_press != mouse_when_up || pointed.size())
                    human->selectEntities(pointed);

            } else if (event.button == SDL_BUTTON_RIGHT) {
                human->moveSelectedInPlane();
                human->updateCenter = true;
            }
        }
    }
    else if(this->gameState == GAME_OVER){
        float y = event.y;
        if(y > 140 && y < 180) gameState = RESET;
        if(y > 180 && y < 220) exit(0);
    }
    else if(this->gameState == MENU){
        float y = event.y;
        if(y > 100 && y < 140) gameState = PLAYING;
        if(y > 140 && y < 180) gameState = RESET;
        if(y > 180 && y < 220) exit(0);
    }
    else if(this->gameState == MAIN_MENU){
        float y = event.y;
        if(y > 180 && y < 220) gameState = PLAYING;
        if(y > 220 && y < 260) gameState = LEVEL;
        if(y > 260 && y < 300) exit(0);
    }
    else if(this->gameState == LEVEL){
        float y = event.y;
        if(y > 100 && y < 140 && selectedLevel != LEVEL_TUTORIAL) {selectedLevel = LEVEL_TUTORIAL; gameState = RESET;}
        if(y > 140 && y < 180 && selectedLevel != LEVEL_NIVEL1) {selectedLevel = LEVEL_NIVEL1; gameState = RESET;}
        if(y > 180 && y < 220 && selectedLevel != LEVEL_DEBUG) {selectedLevel = LEVEL_DEBUG; gameState = RESET;}
        if(y > 220 && y < 260) gameState = MAIN_MENU;
    }

}

void Game::onMouseWheel(SDL_MouseWheelEvent event) {
    //event.x: the amount scrolled horizontally, positive to the right and negative to the left
    //event.y: the amount scrolled vertically, positive away from the user and negative toward the user
    if(this->gameState == PLAYING) {
        human->cameraController->onMouseWheel(event);
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
    GUI::getGUI()->camera2d->setOrthographic(0, window_width, 0, window_height, 0, 1);
}

Player* Game::getTeamPlayer(std::string team){
    if(team == HUMAN_TEAM)
        return this->human;
    else
        return this->enemy;
}

Player* Game::getEnemyTeamPlayer(std::string team){
    if(team != HUMAN_TEAM)
        return this->human;
    else
        return this->enemy;
}

