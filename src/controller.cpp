
#include "controller.h"
#include "game.h"
#include "mesh.h"

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

    switch(mode){
        case 2:
        {
            if(entity == NULL)
                break;

            Vector3 pos = entity->getPosition();
            Vector3 dir = (pos - entityPreviusPos).normalize()*0.5 + entity->getDirection()*0.5;    //TODO Controll velocity
            Game::instance->camera->lookAt(
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

            Game::instance->camera->lookAt(eye,center,up);
        }
        default:
        {
            //FREE CAMERA
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

//================================================

EntityController::EntityController() {}
EntityController::~EntityController() {}

void EntityController::update(double seconds_elapsed, UID e_uid) {
    Entity* entity = Entity::getEntity(e_uid);
    Game* game = Game::instance;
    if(entity == NULL){
        std::cout << "EntityController sin entidad asignada!\n";
        return;
    }

    double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant
    Vector3 dPos(0,0,0);
    if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
    if (game->keystate[SDL_SCANCODE_W]) dPos = dPos + (Vector3(0.0f, 0.0f, -1.0f) * speed);
    if (game->keystate[SDL_SCANCODE_S]) dPos = dPos + (Vector3(0.0f, 0.0f, 1.0f) * speed);
    if (game->keystate[SDL_SCANCODE_A]) dPos = dPos + (Vector3(-1.0f, 0.0f, 0.0f) * speed);
    if (game->keystate[SDL_SCANCODE_D]) dPos = dPos + (Vector3(1.0f, 0.0f, 0.0f) * speed);
    if (game->keystate[SDL_SCANCODE_E]) dPos = dPos + (Vector3(0.0f, 1.0f, 0.0f) * speed);
    if (game->keystate[SDL_SCANCODE_Q]) dPos = dPos + (Vector3(0.0f, -1.0f, 0.0f) * speed);

    Vector3 prev_pos = entity->getPosition();  //TODO change with previus pos (CUIDADO , SI ESTA PARADA LA RESTA DARIA 0)
    entity->model.traslate(dPos.x, dPos.y, dPos.z);
    if (game->keystate[SDL_SCANCODE_SPACE]){
        Vector3 actual_pos = entity->getPosition();
        Vector3 dir =  actual_pos - prev_pos;
        BulletManager::getManager()->createBullet(actual_pos,prev_pos,dir.normalize()*100,100.0f,10.0f,entity->uid,"No type yet");
    }

    std::cout << entity->getPosition().toString() << "\n";

}

//================================================

FighterController::FighterController() {
    acc = 0;
    vel = 0;
    angX = 0;
    angY = 0;
}

FighterController::~FighterController() {

}

void FighterController::update(double seconds_elapsed, UID e_uid) {
    Entity* entity = Entity::getEntity(e_uid);
    Game* game = Game::instance;
    if(entity == NULL){
        //std::cout << "FighterController sin entidad asignada!\n";
        return;
    }
    //Controller mas realista
    //    entity->model.traslate(velocity.x*seconds_elapsed,velocity.y*seconds_elapsed,velocity.z*seconds_elapsed);
    //    velocity = velocity + entity->model.rotateVector(Vector3(0.0,0.0,speed*seconds_elapsed));
    //    velocity = velocity * 0.99; //Friccion
    //
    float dTime = seconds_elapsed; //the dTime is defined by the seconds_elapsed so it goes constant

    if (game->keystate[SDL_SCANCODE_W]) acc = acc + -1.0f;
    if (game->keystate[SDL_SCANCODE_S]) acc = acc + +1.0f;
    if (vel > 0.1) acc += -0.05f;
    else if (vel < -0.1) acc += +0.05f;
    else vel = 0.0f;

    acc *= 0.5;
    vel = vel + acc * dTime * 20;
    //std::cout << "Vel: " << vel << "\tAcc: " << acc << "\n";

    //Bullets:::
    Vector3 prev_pos = entity->getPosition();  //TODO change with previus pos (CUIDADO , SI ESTA PARADA LA RESTA DARIA 0)
    entity->model.traslateLocal(0, 0, vel * dTime);     //TODO change this translate to some velocity vector
    if (game->keystate[SDL_SCANCODE_SPACE]){
        Vector3 actual_pos = entity->getPosition();
        Vector3 dir= entity->getDirection();
        dir.normalize();
        BulletManager::getManager()->createBullet(actual_pos + dir*10,prev_pos + dir*10,dir*300,100.0f,10.0f,entity->uid,"No type yet");
    }
    //End bullets

    if (game->keystate[SDL_SCANCODE_A]) angX += -1.0f;
    if (game->keystate[SDL_SCANCODE_D]) angX += +1.0f;
    if (game->keystate[SDL_SCANCODE_E]) angY += -1.0f;
    if (game->keystate[SDL_SCANCODE_Q]) angY += +1.0f;



    angX = angX * dTime * 0.5;
    angY = angY * dTime * 0.5;
    //std::cout << "angX: " << angX << "\tangY: " << angY << "\n\n";
    entity->model.rotateLocal(angX, Vector3(0,1,0));
    entity->model.rotateLocal(angY, Vector3(1,0,0));
}

//================================================

AIController::AIController() {
    min_dist = MIN_DIST_AI;
}

AIController::~AIController() {

}

void AIController::update(double seconds_elapsed, UID e_uid) {

    Entity* follow = Entity::getEntity(entity_follow);


    if(follow == NULL){
        //std::cout<<"Entity that AI follow is NULL\n";
        return;
    }

    Entity* driving = Entity::getEntity(e_uid);

    Vector3 to_target = follow->getPosition() - driving->getPosition();

    float distance = to_target.length();
    Vector3 looking = driving->getDirection().normalize();
    to_target.normalize();
    float angle = acosf(looking.dot(to_target));
    Vector3 perpendicular = to_target.cross(looking).normalize();

    Matrix44 inv = driving->getGlobalModel();
    inv.inverse();
    Vector3 perpendicularRotate = inv.rotateVector(perpendicular);
    float angleRotate = (angle > 0.03 ? angle * seconds_elapsed : angle);    //Angulo pequeño rota directamente
    if(angleRotate > 0)
        driving->model.rotateLocal(angleRotate,perpendicularRotate);

    if(distance > min_dist)
        driving->model.traslateLocal(0, 0, -100 * seconds_elapsed);     //TODO change this translate to some velocity vector

     //TODO quit this
    Vector3 pos = driving->getPosition();
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+perpendicular*100);
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+looking*100);
    Game::debugMesh.vertices.push_back(pos);
    Game::debugMesh.vertices.push_back(pos+to_target*100);
 }

void AIController::setEntityFollow(UID entity_follow) {
    this->entity_follow = entity_follow;

}
//================================================

ClickController::ClickController() {}
ClickController::~ClickController() {}
void ClickController::update(double seconds_elapsed, UID e_uid) {}
