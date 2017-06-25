
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

void CameraController::update(double seconds_elapsed, UID e_uid) {
    Entity* entity = Entity::getEntity(e_uid);
    Game* game = Game::instance;
    Camera* camera = game->camera;

    switch(mode){
        case 2:
        {
            if(entity == NULL)
                break;

            Vector3 pos = entity->getPosition();
            Vector3 dir = (pos - entityPreviusPos).normalize()*0.5f + entity->getDirection()*0.5f;    //TODO Controll velocity
            camera->lookAt(
                    pos + Vector3(20*(-dir.x), 20*(-dir.y)+10, 20*(-dir.z)),
                    pos,
                    Vector3(0.f, 1.f, 0.f)
            );
            entityPreviusPos = pos*0.05f + entityPreviusPos*0.95f; //TODO hacer interpolación para que poco a poco se vaya poniendo detras

            break;
        }
        case 3:
        {
            //Should only  look relative to entityMesh
            EntityMesh* entityMesh = (EntityMesh*)entity;
            if(entityMesh == NULL)
                break;
            Mesh* mesh = Mesh::Load(entityMesh->mesh);

            Vector3 entityPos = entityMesh->getPosition();

            float entity_radius = mesh->info.radius;

            Vector3 eye = entityPos + Vector3(0,1,0)*entity_radius*10,  //TODO See if x10 distance is ok or should be relative to other thing
                    center = entityPos,
                    up(1,1,0);  //TODO ¿ perpendicular to center - eye ?

            //Vector3 front = center - eye;
            //up = Vector3(0,1, -front.y/front.z).normalize();
            //-(x*v.x + y*v.y)/z = v.z

            camera->lookAt(eye,center,up);
        }
        default:
        {
            //FREE CAMERA
            double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

            //mouse input to rotate the cam
            if ((game->keystate[SDL_SCANCODE_LSHIFT] && (game->mouseLeft)) || game->mouse_locked) //is left button pressed?
            {
                std::vector<Entity*> human_controlling = game->human->getControllingEntities();
                if(!human_controlling.size()) {
                    camera->rotate(game->mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
                    camera->rotate(game->mouse_delta.y * 0.005f,
                                                   camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
                }else if (game->mouse_delta.x || game->mouse_delta.y){
                    
                    Vector3 rotateCenter = game->human->getCenterControlling();
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