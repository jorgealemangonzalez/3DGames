
#include "player.h"
#include "game.h"
#include "mesh.h"
#include "entity.h"

Player::Player(std::string t) : team(t){

}

Player::~Player() {
    for(Entity* e : getControllableEntities()){
        delete e;
    }
}

void Player::addControllableEntity(UID e_uid) {
    controllableEntities.push_back(e_uid);
}

std::vector<Entity*> Player::getControllableEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllableEntities.begin(); it != controllableEntities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllableEntities.erase(it);

    return entities;
}

//========================================

Human::Human() : Player(HUMAN_TEAM) {
    cameraController = new CameraController();
    updateCenter = true;
}

Human::~Human() {
    delete cameraController;
}

void Human::update(double seconds_elapsed) {
    cameraController->update(seconds_elapsed); //TODO Quit entity from camera controller

    std::vector<Entity*> controlling = getControllingEntities();
    Vector3 center;
    for(Entity* e : controlling){
        center += e->getPosition();
    }

    if(updateCenter){
        center /= controlling.size();
        centerControlling = center;
        if(controlling.size()) {
            Camera* camera = Game::instance->camera;
            Vector3 dif = centerControlling - camera->center;
            camera->eye = camera->eye + dif;
            camera->center = centerControlling;
        }
        GUI::getGUI()->setGrid((bool)controllingEntities.size(), centerControlling);
    }
}

void Human::render(Camera *camera) {

}

void Human::centerCameraOnControlling(){
    if(!controllingEntities.size()) return;
    Vector3 dir = Game::instance->camera->eye - centerControlling;
    dir.normalize();
    Camera* camera = Game::instance->camera;
    camera->eye = dir * (radiusControlling+100) + centerControlling;
    camera->center = centerControlling;
}
void Human::selectAllEntities(){
    this->selectEntities(this->controllableEntities);
}
void Human::selectEntities(std::vector<UID>& entities) {
    controllingEntities = entities;
    std::vector<Entity *> controlEntities = getControllingEntities();

    if(controlEntities.size()) {
        Vector3 center;
        for (Entity *e: controlEntities) {
            center += e->getPosition();
        }
        center /= controlEntities.size();
        centerControlling = center;

        Entity *farthest = NULL;
        radiusControlling = 0;
        for (Entity *e: controlEntities) {
            float dist = e->getPosition().distance(centerControlling);
            if (dist >= radiusControlling) {
                radiusControlling = dist;
                farthest = e;
            }
        }

        if(farthest != NULL)
            radiusControlling += Mesh::Load(((EntityMesh *) farthest)->mesh)->info.radius;
    }
}

void Human::organizeSquadCircle(Vector3 position){
    //Intentemos crear un circulo y poner las naves en este
    std::vector<Entity*> controlling = getControllingEntities();
    if(controlling.size()) {
        float acum_sizes = 0.0;
        for (Entity *e : controlling) {
            acum_sizes += Mesh::Load(((EntityMesh *) e)->mesh)->info.radius * 2 + 50;
        }

        //L = 2 * PI * r
        float circ_r = acum_sizes / (2.0 * PI);
        float section = (360.0 / (float) controlling.size());
        for (unsigned int i = 0; i < controlling.size(); ++i) {
            Vector3 move = position +
                           Vector3(circ_r * cos((i * section) * DEG2RAD), 0, circ_r * sin((i * section) * DEG2RAD));
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.vel = controlling[i]->stats.maxvel;
        }
    }
}

void Human::organizeSquadLine(Vector3 position){
    //Intentemos crear un circulo y poner las naves en este
    std::vector<Entity*> controlling = getControllingEntities();
    if(controlling.size()) {
        for (unsigned int i = 0; i < controlling.size(); ++i) {
            int jump = i/2;
            float radius = Mesh::Load(((EntityMesh*)controlling[i])->mesh)->info.radius;
            Vector3 move;
            if(i%2)
                move = position +
                               Vector3( (jump+1)*50 + radius, 0, 0);
            else
                move = position -
                               Vector3( (jump+1)*50 + radius, 0, 0);
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.targetPos = move;
            controlling[i]->stats.vel = 100;
        }
    }
}

bool Human::getPositionSelectedMove(Vector3 &selectedMove) {
    Game* g = Game::instance;
    Camera* camera = g->camera;
    GUI *gui = GUI::getGUI();

    Vector3 O = camera->eye;
    Vector3 A = camera->unproject(Vector2(g->mouse_when_press.x, g->mouse_position.y), g->window_width, g->window_height);
    Vector3 B = camera->unproject(Vector2(g->mouse_when_press), g->window_width, g->window_height);

    Vector3 Bp;
    Vector3 dir = B-O;
    if(!gui->grid->testRayCollision(camera->eye,dir,INFINITY,Bp))
        return false;

    double dist = ( (O-A).length() * (O-Bp).length() ) / (O-B).length();
    Vector3 Ap = O + (A-O).normalize()*dist;
    selectedMove = Ap;
    return true;

}

void Human::moveSelectedInPlane(){
    Vector3 selectedMove;
    if(getPositionSelectedMove(selectedMove))
        organizeSquadCircle(selectedMove);
}

std::vector<Entity*> Human::getControllingEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllingEntities.begin(); it != controllingEntities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllingEntities.erase(it);

    return entities;
}

std::vector<Entity*> Human::getControllableEntities(){
    std::vector<Entity*> entities;
    std::vector< std::vector<UID>::iterator > removeEntities;
    for(std::vector<UID>::iterator it = controllableEntities.begin(); it != controllableEntities.end(); ++it) {
        Entity* entity =Entity::getEntity((*it));
        if (entity != NULL)
            entities.push_back(entity);
        else{
            removeEntities.push_back(it);
        }
    }
    for(std::vector<UID>::iterator it : removeEntities)
        controllableEntities.erase(it);

    return entities;
}

const float &Human::getRadiusControlling() const {
    return radiusControlling;
}

const Vector3 &Human::getCenterControlling() const {
    return centerControlling;
}

//========================================

Enemy::Enemy() : Player(ENEMY_TEAM) {
}

Enemy::~Enemy() {

}

void Enemy::update(double seconds_elapsed) {
    std::vector<Entity*> controllable = getControllableEntities();
    for(Entity* driving: getControllableEntities()){
        for(Entity* ec: Game::instance->human->getControllableEntities()){
            driving->stats.followEntity = ec->uid;
            break;
        }
    }
}