
#include "controller.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"

#define MIN_DIST_AI 100

CameraController::CameraController() : mode(1) {}
CameraController::~CameraController() {}

void CameraController::setMode(int m) { mode = m; }

void CameraController::notifyEntity(UID e_uid) {
    Entity* e = Entity::getEntity(e_uid);
    if(e != NULL)
        entityPreviusPos = e->getGlobalModel().getTranslationOnly();
}

void CameraController::update(double seconds_elapsed) {
    Game* game = Game::instance;
    Camera* camera = game->camera;

    switch(mode){
        default:
        {
            //FREE CAMERA
            double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

            //mouse input to rotate the cam
            std::vector<Entity*> human_controlling = game->human->getControllingEntities();
            if ((game->keystate[SDL_SCANCODE_LSHIFT] && (game->mouseLeft)) || game->mouse_locked) //is left button pressed?
            {

                if (game->mouse_delta.x || game->mouse_delta.y){
                    
                    Vector3 rotateCenter = camera->center;
                    Vector3 lastEye = camera->eye;
                    Vector3 to_entity = lastEye - rotateCenter;
                    bool changeRotation = false;
                    if(to_entity.z < 0)
                        changeRotation = true;
                    //std::cout<<"Vector: "<<to_entity;
                    Matrix44 rotateYaw, rotatePitch;
                    rotateYaw.setRotation(-game->mouse_delta.x * 0.005f, Vector3(0.0f, 1.0, 0.0f));
                    Vector3 up = Vector3(0,1,0);
                        //TODO Controlar que no llegue nunca a ser cero el cross !PELIGROSO /0
                    Vector3 rightVector = up.cross(to_entity);//rotateYaw.rotateVector( Vector3(1.0f, 0.0f, 0.0f) )
                    rotatePitch.setRotation(game->mouse_delta.y * 0.005f,rightVector);
                    //rotatePitch.setRotation(game->mouse_delta.y * 0.005f,Vector3(1,0,0) );

                    Vector3 rotated_to_entity = (rotatePitch*rotateYaw).rotateVector(to_entity);

                    camera->eye = rotateCenter + rotated_to_entity;
                }
            }
            //Arrastrar camara
            if(game->mouseRight && !human_controlling.size()){
                float maxDist = 0;
                for(auto it = Entity::s_entities.begin() ; it != Entity::s_entities.end(); ++it){
                    float dist = it->second->getPosition().distance(camera->eye);
                    maxDist = MAX(maxDist,dist);

                }
                maxDist = MIN(10000.0,maxDist);
                maxDist -= 3000.0;
                maxDist = MAX(0,maxDist);


                Vector3 right = (camera->center - camera->eye).normalize().cross(camera->up);
                double delta_x = game->mouse_delta.x*(maxDist*maxDist/1000000.0);
                double delta_y = game->mouse_delta.y*(maxDist*maxDist/1000000.0);
                camera->eye = camera->eye + right*delta_x +camera->up.normalize()*delta_y;
                camera->center = camera->center + right*delta_x +camera->up.normalize()*delta_y;
            }


            //async input to move the camera around or if mouse in window edge
            bool pasiveMove = !(game->mouseLeft || game->mouseRight);
            if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
            if (game->keystate[SDL_SCANCODE_UP] || (pasiveMove && game->mouse_position.y > game->window_height-game->window_height*0.1))
                camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
            if (game->keystate[SDL_SCANCODE_DOWN] || (pasiveMove && game->mouse_position.y < game->window_height*0.1))
                camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
            if (game->keystate[SDL_SCANCODE_LEFT] || (pasiveMove && game->mouse_position.x < game->window_width*0.1))
                camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
            if (game->keystate[SDL_SCANCODE_RIGHT] || (pasiveMove && game->mouse_position.x > game->window_width-game->window_width*0.1))
                camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

            //to navigate with the mouse fixed in the middle
            if (game->mouse_locked) {
                int center_x = (int) floor(game->window_width * 0.5f);
                int center_y = (int) floor(game->window_height * 0.5f);
                //center_x = center_y = 50;
                SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen
                //SDL_WarpMouseGlobal(center_x, center_y); //put the mouse back in the middle of the screen

                game->mouse_position.x = (float) center_x;
                game->mouse_position.y = (float) center_y;
            }


            Game::instance->angle += (float) seconds_elapsed * 10.0f;
            break;
        }
    }
}

void CameraController::onMouseWheel(SDL_MouseWheelEvent event){
    Camera* camera = Game::instance->camera;

    Vector3 delta;
    if (event.y > 0 && (camera->eye.distance(camera->center) > 200)){
        delta = Vector3(0.0f, 0.0f, 1.0f) * event.y*100.f;
    }
    if (event.y < 0){
        delta = Vector3(0.0f, 0.0f, -1.0f) * abs(event.y)*100.f;
    }

    if(delta){
        Vector3 localDelta = camera->getLocalVector(delta);
        camera->eye = camera->eye - localDelta;
        camera->updateViewMatrix();
    }
}