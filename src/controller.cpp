
#include "controller.h"
#include "game.h"

CameraController::CameraController() {
    mode = 1;
    entity = NULL;
}

CameraController::~CameraController() {}

void CameraController::setEntity(Entity* e) {
    entity = e;
}

void CameraController::setMode(int m) {
    mode = m;
}

void CameraController::update(double seconds_elapsed) {
    switch(mode){
        case 2:
        {
            if(entity != NULL){
                Vector3 pos = entity->getPosition();
                Vector3 dir = entity->getRotation();

                Game::instance->camera->lookAt(pos - dir, pos, Vector3(0.f, 1.f, 0.f));
            }
        }
            break;
        case 1:
        default:
        {
            double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

            //mouse input to rotate the cam
            if ((Game::instance->mouse_state & SDL_BUTTON_LEFT) || Game::instance->mouse_locked) //is left button pressed?
            {
                Game::instance->camera->rotate(Game::instance->mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
                Game::instance->camera->rotate(Game::instance->mouse_delta.y * 0.005f, Game::instance->camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
            }

            //async input to move the camera around
            if (Game::instance->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
            if (Game::instance->keystate[SDL_SCANCODE_UP]) Game::instance->camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
            if (Game::instance->keystate[SDL_SCANCODE_DOWN]) Game::instance->camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
            if (Game::instance->keystate[SDL_SCANCODE_LEFT]) Game::instance->camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
            if (Game::instance->keystate[SDL_SCANCODE_RIGHT]) Game::instance->camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

            //to navigate with the mouse fixed in the middle
            if (Game::instance->mouse_locked) {
                int center_x = (int) floor(Game::instance->window_width * 0.5f);
                int center_y = (int) floor(Game::instance->window_height * 0.5f);
                //center_x = center_y = 50;
                SDL_WarpMouseInWindow(Game::instance->window, center_x, center_y); //put the mouse back in the middle of the screen
                //SDL_WarpMouseGlobal(center_x, center_y); //put the mouse back in the middle of the screen

                Game::instance->mouse_position.x = (float) center_x;
                Game::instance->mouse_position.y = (float) center_y;
            }


            Game::instance->angle += (float) seconds_elapsed * 10.0f;
            break;
        }
    }
}

EntityController::EntityController() {
    entity = NULL;
}

EntityController::~EntityController() {

}

void EntityController::setEntity(Entity *e) {
    this->entity = e;
}

void EntityController::update(double seconds_elapsed) {
    if(entity == NULL){
        std::cout << "EntityController sin entidad asignada!\n";
        return;
    }

    double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant
    Vector3 dPos(0,0,0);
    if (Game::instance->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
    if (Game::instance->keystate[SDL_SCANCODE_W]) dPos = dPos + (Vector3(0.0f, 0.0f, 1.0f) * speed);
    if (Game::instance->keystate[SDL_SCANCODE_S]) dPos = dPos + (Vector3(0.0f, 0.0f, -1.0f) * speed);
    if (Game::instance->keystate[SDL_SCANCODE_A]) dPos = dPos + (Vector3(1.0f, 0.0f, 0.0f) * speed);
    if (Game::instance->keystate[SDL_SCANCODE_D]) dPos = dPos + (Vector3(-1.0f, 0.0f, 0.0f) * speed);
    entity->model.traslate(dPos.x, dPos.y, dPos.z);
    std::cout<<"Translation: "<<entity->model.getTranslationOnly().toString()<<"\n";
}
